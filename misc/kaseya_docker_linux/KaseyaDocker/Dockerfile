FROM debian:8

#
# This is a systemd-enabled container that runs in unprivileged mode.
#
# It will run the Kaseya setup the first time it runs then start the agent.
# When relaunched subsequently it'll start the agent without repeating setup.
#
# First build the image:
#
#     docker build -t kaseya:latest .
#
# Then if you have SELinux, enable the selinux boolean container_manage_cgroup
# permanently with:
#
#     sudo setsebool -P container_manage_cgroup 1
#
# Then run the container:
#
#     docker run --name kaseya -it --restart unless-stopped --cap-add CAP_SYS_ADMIN --stop-signal RTMIN+3 --tmpfs /tmp --tmpfs /run --tmpfs /run/lock -v /sys/fs/cgroup:/sys/fs/cgroup:ro --env oci-systemd-hook=disabled kaseya:latest
#
# If all goes well it should print output like:
#
#     [*     ] A start job is running for Setup Kaseya (5s / no limit)
#     [  OK  ] Stopped LSB: Kaseya Agent.
#     [  OK  ] Reached target Network is Online.
#     	 Starting LSB: Kaseya Agent...
#     [  OK  ] Started LSB: Kaseya Agent.
#     [  OK  ] Started Setup Kaseya.
#
# To detach use CTRL-P CTRL-Q.
#
# To run commands inside the live container use 'docker container exec' e.g.:
#
#    docker container exec kaseya systemctl -a
#
# For an interactive shell:
#
#    docker container exec -it kaseya /bin/bash
#
# To check installation status for the agent:
#
#    docker container exec kaseya journalctl -u install-kaseya.service
#
#    docker container exec kaseya systemctl status install-kaseya.service
#
# It's normal for this to show a few warnings in the logs, but you should see
# code=exited, status=0/SUCCESS, something like:
#
#    * install-kaseya.service - Setup Kaseya
#        Loaded: loaded (/etc/systemd/system/install-kaseya.service; enabled)
#        Active: active (exited) since Mon 2021-02-01 08:43:58 UTC; 4min 41s ago
#      Main PID: 164 (code=exited, status=0/SUCCESS)
#
# The agent doesn't use native systemd, it uses sysv compatibility, so you have
# to use the /etc/init.d/kagent-* script as managed by systemd to control it. So
# you won't find a services file.
#
# To check run state for the agent service once installed:
#
#    docker container exec kaseya journalctl -u 'kagent-*.service'
#
#    docker container exec kaseya systemctl status --all  'kagent-*.service'
#
# Privileged mode not recommended
# ===============================
#
# Do not pass the --privileged flag when running this container. This container
# must run with uid and gid 0, i.e. docker's root, so it's only protected by
# the limited isolation provided by namespaces, capability flags, etc. It's
# almost certainly possible to break out of this container with only limited
# effort, but it's better than no isolation. Unfortunately, it requires CAP_SYS_ADMIN
# for d-bus to work.
#
# NOTES ON SYSTEMD
# -----
# Ideally the host Docker daemon (or containerd and runc) would take care of
# the necessary tmpfs mounts, etc, for a systemd-enabled container using OCI
# hooks like oci-systemd-hook in the Docker host. See
# https://github.com/projectatomic/oci-systemd-hook and
# https://developers.redhat.com/blog/2016/09/13/running-systemd-in-a-non-privileged-container/
#
# Unfortunately Docker CE does not support running OCI hooks. runc does and so
# does the patched Docker on RHEL and Fedora. For regular Docker CE you need to
# do some extra steps, e.g.:
#
# Note: If you are on an OCI hooks enabled build but want to suppress the
# oci-systemd-hook that automounts /tmp etc, add:
#
#     --env oci-systemd-hook=disabled
#
# TROUBLESHOOTING
# ----
#
# init.scope permission denied
# =======
#
# If you get an error like:
#
#    Failed to create /system.slice/docker-af56aa4c5817896c222a0e8b59c2f1c39f8de07856f67ab9b470126420097245.scope/init.scope control group: Permission denied
#    Failed to allocate manager object: Permission denied
#
# ... then you may also need to allow the container to manage control groups
# with:
#
#     setsebool container_manage_cgroup 1
#
# This can be set persistently with the -P flag to sesetbool. Be sure you understand
# the implications since this weakens the already weak sandboxing Docker provides
# for unprivileged containers.
#
# Failed to create root cgroup hierarchy: Permission denied
# =======
#
# Same as above:
#
#    setsebool container_manage_cgroup 1
#
# Kaseya service not running
# ========
#
# See notes below on how to observe processes, get access to journalctl for
# logs, etc.
#
# You can always "docker exec -it /bin/bash {{containername}}" to get a shell.
#
# Container doesn't respond to SIGTERM or SIGINT and won't exit interactively
# ========
#
# See comments below on STOPSIGNAL. Use "docker kill" instead.
#

ENV DEBIAN_FRONTEND noninteractive

# Kaseya's installer needs perl and MIME::Base64 which is not part of the debian
# minimalist perl that's included in the base docker image.
RUN apt-get -y update && apt-get -y install perl

#
# This image won't shutdown on SIGTERM. It expects the custom stop signal
# RTMIN+3 per systemd requirements. 'docker stop' and 'docker kill' know how to
# use it if declared here.
#
# Systemd won't respond to SIGINT either, so control-C won't stop it if
# you launch it interactively.
#
STOPSIGNAL RTMIN+3

#
# Run a minimal systemd only, don't try to bring up a full system.
#
ENV SYSTEMD_COLORS=0
COPY systemctl-commands.sh /root/systemctl-commands.sh
RUN bash /root/systemctl-commands.sh && rm -f /root/systemctl-commands.sh
RUN echo "NAutoVTs=0" >> /etc/systemd/logind.conf

#
# Run Kaseya setup on first startup.
#
# We can't run it during "docker build" because it requires systemd to be
# running. The install-kaseya.service task will self-disable when successful.
#
ADD KcsSetup.sh /root/KcsSetup.sh
ADD KcsSetupSuccessful.sh /root/KcsSetupSuccessful.sh
RUN chmod a+x /root/KcsSetup.sh /root/KcsSetupSuccessful.sh
ADD install-kaseya.service /etc/systemd/system/
# This works when systemd isn't running because it just makes symlinks
RUN systemctl enable install-kaseya.service

# See selinux/ dir
# This won't work: semanage needs the real context store, and chcon
# is not persistent. How to do? Write a policy that detects it?
#RUN apt-get -y update && apt-get -y install selinux-basics
#RUN semanage fcontext -m -t container_init_t /lib/systemd/systemd
#RUN ls -lZ /lib/systemd/systemd

#
# Run systemd when this container is launched with "docker run"
#
# Handy hints for observing the container:
#
# List units
#
#    docker exec -it $CONTAINERID systemctl -a -l --no-pager
#
# List processes
#
#    docker exec -it ps aux
#
# Observe container start/stop etc: run "docker events" in another terminal.
#
# See https://www.freedesktop.org/software/systemd/man/systemd.html for systemd env vars etc.
#
# If you want more info on systemd startup you can add --show-status to the command line.
#
CMD ["/lib/systemd/systemd", "--system", "--log-level=debug", "--unit=multi-user.target"]
