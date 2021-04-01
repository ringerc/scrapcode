#!/bin/bash
set -e -u -x
GUID=${GUID:-KSAASS64114577563164}

# TODO should have separate setup service
sudo systemctl stop kaseya.service || true
sudo auditctl -d never,task || true
setupexe=/opt/Kaseya/KcsSetup
if [ -e $setupexe ]; then
    sudo auditctl -A always,exit -F exe=$setupexe -F arch=b32 -S all -k kcs
    sudo auditctl -A always,exit -F exe=$setupexe -F arch=b64 -S all -k kcs
    sudo auditctl -A never,exit -F exe=$setupexe -F dir=/opt/Kaseya -k kcs
    sudo auditctl -A always,exit -F exe=$setupexe -F dir=/ -k kcs
fi
agent=/opt/Kaseya/${GUID}/AgentMon
if [ -e $agent ]; then
    # apparently auditctl can't target procs for files that don't exist yet
    # and it seems it doesn't filter by cgroup either, so only check the agent
    # if it exists.
    sudo auditctl -A always,exit -F exe=$agent -F arch=b32 -S all -k kcs
    sudo auditctl -A always,exit -F exe=$agent -F arch=b64 -S all -k kcs
    sudo auditctl -A never,exit -F exe=$agent -F dir=/opt/Kaseya -k kcs
    sudo auditctl -A always,exit -F exe=$agent -F dir=/ -k kcs
fi
sudo systemctl start kaseya.service
#sleep 60
sleep 10
sudo systemctl stop kaseya.service
sudo auditctl -D -k kcs
sudo auditctl -a never,task

sudo ausearch --format raw -ts recent -k kcs | aureport --syscall -i --summary > kaseya-syscalls.report || true
sudo ausearch --format raw -ts recent -k kcs | aureport --file -i --summary > kaseya-files.report || true
