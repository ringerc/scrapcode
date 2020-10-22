"""
Use strongest available credentials store in simple interface.

Will use libsecret if available, otherwise falls back to a yaml file with
permissions restrictions and safe overwrite.

Usage:

    import logging
    import secret_store

    def main():
        logging.basicConfig(level='INFO')
        secret_store.initialize("my_app_name")

        secret_store.store("key", "secretwhisper")
        whispers = secret_store.lookup("key")
"""

import os
import logging
import yaml

logger = logging.getLogger(__name__)

class BaseSecretLookup:

    def __init__(self, description=None, can_store = True):
        if description is None:
            description = "provider " + self.name
        self.description = description
        self._can_store = can_store

    def _check_lookup(self, key):
        if not isinstance(key, str):
            raise TypeError("key must be 'str', but got key type {}".format(type(key)))
        if not key:
            raise ValueError("key must be non-empty string")

    def _check_store(self, key, secret):
        if not isinstance(key, str) or not isinstance(secret, str):
            raise TypeError("key and secret must be strings, but got key {} secret {}".format(type(key), type(secret)))
        if not key:
            raise ValueError("key must be non-empty string")
        if not secret:
            raise ValueError("secret must be non-empty string")

    @classmethod
    def is_usable(self):
        """Can this provider be used?"""
        return True

    @property
    def can_store(self):
        """Can this provider store new secrets?"""
        return self._can_store

class EnvLookup(BaseSecretLookup):

    name = "envvar"

    def __init__(self, appname):
        super().__init__(description="envirionment variables", can_store=False)

    def lookup(self, key):
        """Dummy 'secret' lookup that checks env-vars"""
        super()._check_lookup(key)
        try:
            return os.environ["key"]
        except KeyError:
            pass
        return None

    def store(self, key, secret):
        super()._check_store(key, secret)
        raise NotImplementedError("No support for storing creds in environment")

def atomic_overwrite(destpath, newcontent, mode=0o0600):
    """
    Atomically overwrite file at destpath with newcontent in a crashsafe way so
    that stupid file systems don't leave us with an empty file and lost
    original contents.
    """
    tmppath = os.path.join(os.path.dirname(destpath), "." + os.path.basename(destpath) + ".tmp")
    if os.path.exists(tmppath):
        os.unlink(tmppath)
    try:
        tmpfile = os.fdopen(os.open(tmppath, os.O_WRONLY|os.O_CREAT|os.O_EXCL, mode),"w")
        assert((os.stat(tmppath).st_mode & 0o777) & (~ mode) == 0)
        tmpfile.write(newcontent)
        tmpfile.flush()
        os.fsync(tmpfile.fileno())
        tmpfile.close()
        dirfd = os.open(os.path.dirname(tmppath), os.O_DIRECTORY, 0)
        os.fsync(dirfd)
        os.rename(tmppath, destpath)
        os.fsync(dirfd)
        os.close(dirfd)
    finally:
        try:
            os.unlink(tmppath)
        except OSError:
            pass

class YamlConfLookup(BaseSecretLookup):

    name = "yaml"

    def __init__(self, appname):
        self.credfile = os.path.expanduser("~/.cache/{appname}/{appname}_cred.yml".format(appname=appname))
        super().__init__(description = "yaml file {}".format(self.credfile))

    def _check_credfile_permissions(self):
        creddir = os.path.dirname(self.credfile)
        if os.path.exists(creddir) and os.stat(creddir).st_mode & 0o0066 > 0:
            raise RuntimeError("creds dir at {} is group or world readable or writeable, refusing to use it".format(creddir))
        if os.path.exists(self.credfile) and os.stat(self.credfile).st_mode & 0o0066 > 0:
            raise RuntimeError("creds file at {} is group or world readable or writeable, refusing to use it".format(self.credfile))

    def lookup(self, key):
        """Dummy 'secret' lookup that checks a creds file"""
        super()._check_lookup(key)
        self._check_credfile_permissions()
        if os.path.exists(self.credfile):
            credmap = yaml.load(open(self.credfile,"r"), Loader=yaml.SafeLoader)
            if key in credmap:
                return credmap[key]
        return None

    def store(self, key, secret):
        super()._check_store(key, secret)
        os.makedirs(os.path.dirname(self.credfile), mode=0o0700, exist_ok=True)
        self._check_credfile_permissions()
        if os.path.exists(self.credfile):
            credmap = yaml.load(open(self.credfile,"r"), Loader=yaml.SafeLoader)
        else:
            credmap = {}
        credmap[key] = secret
        atomic_overwrite(self.credfile, yaml.dump(credmap, Dumper=yaml.SafeDumper))
        self._check_credfile_permissions()

class LibsecretLookup(BaseSecretLookup):
    name = "libsecret"

    # For loading libsecret support dynamically
    _have_libsecret = None
    _Secret = None

    @classmethod
    def _import_libsecret(klass):
        """
        Import libsecret support if possible, and set _have_libsecret. If
        loaded, set _Secret to the gi.repository.Secret module.
        """
        if klass._have_libsecret is None:
            try:
                import gi
                gi.require_version('Secret', '1')
                from gi.repository import Secret
                klass._Secret = Secret
                klass._have_libsecret = True
            except ImportError:
                logger.debug("cannot import libsecret: {}", exc_info=True)
                klass._have_libsecret = False

    def __init__(self, appname):
        super().__init__(description = 'login keyring (libsecret)')
        self._import_libsecret()
        if not self._have_libsecret:
            raise RuntimeError("libsecret support not available")
        Secret = self._Secret

        self._secret_schema = Secret.Schema.new(
                "com.2ndquadrant.{}.Store".format(appname),
                Secret.SchemaFlags.NONE,
                { "key": Secret.SchemaAttributeType.STRING }
            )

    def lookup(self, key):
        """Use libsecret without schema qualification to look up the default set"""
        super()._check_lookup(key)
        Secret = self._Secret
        found_secret = Secret.password_lookup_sync(self._secret_schema, {'key': key}, None)
        # if we didn't find it we'll just return None
        return found_secret

    def store(self, key, secret):
        """store in libsecret"""
        super()._check_store(key, secret)
        Secret = self._Secret
        Secret.password_store_sync(self._secret_schema, {'key': key}, Secret.COLLECTION_DEFAULT, "tpacheck: {}".format(key), secret, None)

    @classmethod
    def is_usable(klass):
        klass._import_libsecret()
        return klass._have_libsecret

candidate_providers = [LibsecretLookup, YamlConfLookup, EnvLookup]
active_providers = []

def initialize(appname):
    """Set up the module by specifying an application name"""
    global active_providers
    logger.debug("initialising with appname {}".format(appname))
    for provider in candidate_providers:
        if provider.is_usable():
            logger.debug("initialsing provider {}".format(provider.name))
            active_providers.append(provider(appname))
            logger.debug("initialised")
        else:
            logger.debug("skipping provider {}: reports not usable".format(provider.name))
    logger.debug("initialised")

def check_initialized():
    if len(active_providers) == 0:
        raise RuntimeError("call secrets.initialize(...) first")

def lookup_secret(key):
    """Find secret and return it. This is the main module interface."""
    check_initialized()
    first_found = None
    found_locations = []
    logger.debug("Looking for secret {}".format(key))
    for provider in active_providers:
        logger.debug("Trying {}".format(provider.name))
        found = provider.lookup(key)
        if found is not None:
            logger.debug("Found in {}".format(provider.name))
            if first_found is None:
                first_found = found
            found_locations.append(provider.name)
    if len(found_locations) == 0:
        raise KeyError("Could not find secret {} in configured sources: {}".format(key, ', '.join([x.name for x in active_providers])))
    elif len(found_locations) > 1:
        logger.warning("Found same secret in multiple configured sources: {}".format(key, ', '.join([x.name for x in active_providers])))
    return first_found

def store_secret(key, secret):
    """Store secret in first provider available. This is the main module store interface."""
    check_initialized()
    # Should do smart stuff like find if cred exists in other provider and complain etc
    # but in reality we should probably only use one provider anyhow.
    provider=active_providers[0]
    logger.debug("Storing secret {} in provider {}".format(key, provider.name))
    provider.store(key, secret)
    logger.debug("Checking round-trip retrieval of secret")
    if provider.lookup(key) != secret:
        raise RuntimeError("failed to store secret {} in provider {}: retrieval after store failed".format(key, provider.name))
    logger.debug("Stored")

def selftest():
    logging.basicConfig(level='DEBUG')
    initialize("tpacheck")

    store_secret('TEST_TPACHECK_SECRET', 'notverysecret');
    assert(lookup_secret('TEST_TPACHECK_SECRET') == 'notverysecret')

    for provider in active_providers:
        (k, v) = ('TEST_TPACHECK_SECRET_' + provider.__class__.__name__, 'notverysecret_' + provider.__class__.__name__)
        if provider.can_store:
            provider.store(k, v)
            assert(provider.lookup(k) == v)

if __name__ == '__main__':
    selftest()
