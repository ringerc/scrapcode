#!/bin/bash

set -e -u

MAVEN=mvn
ASADMIN=$HOME/java/glassfish3/bin/asadmin
DOMAIN=domain1
DOMAIN_URL=http://localhost:8080/
WAR_ARCHIVE=target/leaktest2.war
# Context root should not have a leading slash
CONTEXT_ROOT=com.mycompany_leaktest2_war_1.0-SNAPSHOT
DEPLOY_NAME=$CONTEXT_ROOT

if ! test -e pom.xml; then
        echo "You're not running this script from a test case directory."
        echo "Please read the README for correct usage."
        exit 1
fi

if ! test -x "$MAVEN" -o -x "`type -pf "$MAVEN"`"; then
        echo "Cannot find maven. Check the setting for the MAVEN variable"
        echo "at the top of test.sh ."
        echo "Looked for maven as: \"$MAVEN\""
        exit 1
fi

if ! test -x "$ASADMIN" -o -x "`type -pf "$ASADMIN"`"; then
        echo "Cannot find asadmin. Check the setting for the ASADMIN variable"
        echo "at the top of test.sh"
        echo "Looked for asadmin as: \"$ASADMIN\""
        exit 1
fi

JHAT_ARGS="-J-Xmx2000m -port 7000"

# Clean up any leftovers from a prior run
"$ASADMIN" start-domain "$DOMAIN" >&/dev/null ||true
"$ASADMIN" undeploy "$DEPLOY_NAME" >&/dev/null ||true

# Check to see if the server is clean
if ! "$ASADMIN" list-applications |grep -q 'Nothing to list' ; then 
	echo "Test instance contains deployed applications, cannot proceed."
	echo "Undeploy all applications on the server and try again."
	echo "Deployed applications are:"
	"$ASADMIN" list-applications -l
fi

# Restart it to make sure any leaked stuff after cleanup is gone
"$ASADMIN" stop-domain "$DOMAIN"
"$ASADMIN" start-domain "$DOMAIN"

# Rebuild the package
"$MAVEN" clean package > build.log

# Deploy, make a test request to the index page and undeploy
echo "Deploying archive"
"$ASADMIN" deploy \
	--name "$DEPLOY_NAME" \
	--contextroot "$CONTEXT_ROOT" \
	"$WAR_ARCHIVE"
echo "Fetching main page..."
if ! curl -s -f $DOMAIN_URL/$CONTEXT_ROOT >/dev/null; then
	echo "Main page fetch FAILED. Aborting run."
fi
echo "Main page fetched, undeploying"
"$ASADMIN" undeploy "$DEPLOY_NAME"

# As of this point there should be no com.mycompany classes in Glassfish's memory

# Dump memory and start jhat
rm -f leak-test ||true
jmap -dump:format=b,file=leak-test $(jps|grep '\(glassfish\|ASMain\)' |cut -d ' ' -f 1)
"$ASADMIN" stop-domain "$DOMAIN"

echo "After jhat finishes starting, run leak-report.sh then control-C to exit jhat"
jhat $JHAT_ARGS leak-test
