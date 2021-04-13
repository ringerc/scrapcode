#!/bin/bash

set -e -u

set -x

rm -rf kcs
mkdir kcs

# Find the embedded base64 encoded KcsSetup in the
# KcsSetup.sh and unpack it. Then append the trailer
# section of KcsSetup.sh, which contains extra "extractor items"
# that KcsSetup expects to find.
#
awk '
BEGIN {
    b64cmd = "base64 -d > kcs/KcsSetup"
}
/^# {{{/ {
	extract_setup = 1
	printf "Extracting KcsSetup...."
	next
}
/^# }}}/ {
	extract_setup = 0
	fflush(b64cmd)
	close(b64cmd)
	printf "done\n"
	next
}
extract_setup == 1 {
	print | b64cmd
}
/^# Begin Extraction 2:$/ {
    extracting_2 = 1
    next
}
extracting_2 {
    print >> "kcs/KcsSetup"
}
END {
    close("kcs/KcsSetup")
}
' KcsSetup.sh

# KcsSetup uses a weird homebrew archive format where chunks of data end with
# a "# Starting FileExtractorItem" with a little metadata in it. Or the
# FileExtractorItem may instead have an Offset: key, in which case we have to seek
# within KcsSetup to find the real data.
#
# Manual extraction.
#
# We really only require KcsSetup itself (with the bits and bobs from
# KcsSetup.sh appended to it) and the KcsSetup_Args file now. But might as well
# extract everything.
#
awk '
# Seek past the executable chunk to find the extractor items section
/^# Starting Embedded Installer Files$/ {
    extracting = 1
    next
}
! extracting { next }

/^# Starting FileExtractorItem:/ {
        itemheader = 1
        filename=""
        size=0
        offset=0
        next
}
itemheader && /^Type:/ {
        filename = $2
        next
}
itemheader && /^Size:/ {
        next
}
itemheader && /^Offset:/ {
        next
}
itemheader && /^File:/ {
        next
}
/^# End of FileExtractorItem:/ {
        close("kcs/temp.item")
        system("mv kcs/temp.item " "kcs/" filename)
        itemheader = 0
        # We could check "size" here, but meh
        next
}
itemheader {
        printf("unrecognised item line %s\n", $0)
        exit
}

# Item payloads come before the "Starting FileExtractorItem" marker,
# so we accumulate them into a tempfile before we find out what they
# should be called.
extracting && !itemheader { print > "kcs/temp.item" }
' kcs/KcsSetup

# KaSetup_Args /a argument is the guid
read -r KCS_SETUP_ARGS < kcs/KaSetup_Args
guid=${KCS_SETUP_ARGS##*/a=}
guid=${guid% *}


sudo mkdir -p /opt/Kaseya
sudo cp kcs/KcsSetup /opt/Kaseya
sudo chmod +x /opt/Kaseya/KcsSetup

# This systemd service definition sandboxes the Kaseya installer and the
# runtime agent into a confined runtime environment with minimum privileges.
#
# While it runs as uid 0, it has none of the special privileges of root and all
# means of privilege elevation are disabled. Unnecessary system calls are
# filtered so they'll return -EPERM if attempted. Most of the filesystem is
# hidden from the agent, with select parts exposed read-only. It only has
# read/write within its own /opt/Kaseya directory. Loading of kernel modules
# and modification of sysfs tunables or procfs sysctls is explicitly blocked.
# Access to hardware device nodes, any direct hardware I/O, etc is blocked.
#
# To understand this you'll want to see manpages:
#
#   systemd.unit(5)
#   systemd.service(5)
#   systemd.exec(5) heading "SANDBOXING"
#   systemd.resource-control(5)
#   capabilities(7)
#   cgroups(7)
#   namespaces(7)
#   https://www.kernel.org/doc/html/latest/userspace-api/no_new_privs.html
#
cat > kaseya.service <<__END__
[Service]
ExecStartPre=-/opt/Kaseya/KcsSetup $(sed 's~/~-~g' < kcs/KaSetup_Args) -i -x -p /opt/Kaseya
ExecStart=/opt/Kaseya/${guid}/bin/AgentMon
# see systemd.exec(5), systemd.resource-control(5)
RestrictNamespaces=true
RestrictSUIDSGID=true
SecureBits=noroot noroot-locked
NoNewPrivileges=yes
AmbientCapabilities=
CapabilityBoundingSet=
ProtectHostname=true
ProtectClock=true
ProtectKernelTunables=true
ProtectKernelModules=true
ProtectControlGroups=true
ProtectHome=tmpfs
DevicePolicy=closed
DeviceAllow=/dev/log
#ProtectSystem=strict
ProtectSystem=full
#ReadWritePaths=/opt/Kaseya
#ReadWritePaths=/run
#ReadWritePaths=/var/run
#ReadWritePaths=/tmp
# Required for PrivateTmp
#ReadWritePaths=/var/tmp
#ReadOnlyPaths=/etc
#ReadOnlyPaths=/dev
#ReadOnlyPaths=/proc
#ReadOnlyPaths=/sys
#ReadOnlyPaths=/lib
#ReadOnlyPaths=/bin
#ReadOnlyPaths=/sbin
#ReadOnlyPaths=/usr/lib
#ReadOnlyPaths=/usr/bin
#ReadOnlyPaths=/usr/sbin
#TemporaryFileSystem=/etc/profile.d
PrivateTmp=true
PrivateUsers=true
# See systemd-analyse syscall-filter, and systemd.exec(5)
#SystemCallFilter=@default @process @basic-io @chown @file-system @network-io @timer
#SystemCallFilter=~@mount @module @privileged @reboot @debug @keyring @setuid
#SystemCallFilter=@system-service
SystemCallFilter=~@mount
__END__

sudo cp kaseya.service /etc/systemd/system/
sudo systemctl --system daemon-reload
sudo systemctl start kaseya.service

sleep 5

systemctl status kaseya.service
#systemd-cgls -u kaseya.service
#journalctl -u kaseya.service
