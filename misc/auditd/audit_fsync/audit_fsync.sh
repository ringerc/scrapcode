#!/bin/bash
sudo auditctl -a exit,always -F arch=b64 -S fsync -k audit_fsync
