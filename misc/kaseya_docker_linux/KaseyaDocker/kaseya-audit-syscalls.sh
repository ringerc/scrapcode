#!/bin/bash
sudo systemctl stop kaseya.service
sudo auditctl -d never,task
sudo auditctl -A always,exit -F arch=b32 -S all -F exe=/opt/Kaseya/KSAASS64114577563164/bin/AgentMon -k kcs
sudo auditctl -A always,exit -F arch=b64 -S all -F exe=/opt/Kaseya/KSAASS64114577563164/bin/AgentMon -k kcs
sudo auditctl -A never,exit -F exe=/opt/Kaseya/KSAASS64114577563164/bin/AgentMon -F dir=/opt/Kaseya -k kcs
sudo auditctl -A always,exit -F exe=/opt/Kaseya/KSAASS64114577563164/bin/AgentMon -F dir=/ -k kcs
sudo systemctl start kaseya.service
sleep 60
sudo systemctl stop kaseya.service
sudo auditctl -d -k kcs
sudo auditctl -a never,task
sudo systemctl start kaseya.service

sudo ausearch --format raw -ts recent -k kcs | aureport --syscall -i --summary > kaseya-syscalls.report
sudo ausearch --format raw -ts recent -k kcs | aureport --file -i --summary > kaseya-files.report
