#!/bin/bash
set -e -x -u
shopt -s nullglob

handle_err() {
    echo "Trapped error ${errcode}, bailing out"
    exit ${errcode}
}
trap 'errcode=$?; handle_err' ERR

kagent=(/etc/init.d/kagent*)
if [ -z "${kagent}" ]; then
    echo "KcsSetup.sh does not appear to have created an initscript, aborting"
    exit 1
fi

echo "KcsSetup.sh created ${kagent}"

# Make sure systemd finds the initscript
systemctl daemon-reload

# Ensure the service runs
systemctl enable ${kagent##*/}.service
systemctl start ${kagent##*/}.service

# don't run again
systemctl disable install-kaseya.service

# Make really sure we don't run again and it's obvious
# the job is done.
mv /root/KcsSetup.sh /root/KcsSetup.sh.disabled
