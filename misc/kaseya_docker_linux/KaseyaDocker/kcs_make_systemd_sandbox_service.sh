#!/bin/bash

set -e -u

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

cat > kaseya.service <<__END__
[Service]
ExecStartPre=-/opt/Kaseya/KcsSetup $(sed 's~/~-~g' < kcs/KaSetup_Args) -i -x -p /opt/Kaseya
ExecStart=/opt/Kaseya/${guid}/bin/AgentMon
AmbientCapabilities=
CapabilityBoundingSet=
ProtectSystem=full
ProtectHome=tmpfs
ReadWritePaths=/opt/Kaseya
SystemCallFilter=~@mount
TemporaryFileSystem=/etc/profile.d
PrivateTmp=true
PrivateUsers=true
ProtectHostname=true
ProtectClock=true
ProtectKernelTunables=true
ProtectKernelModules=true
ProtectControlGroups=true
RestrictNamespaces=true
RestrictSUIDSGID=true
NoNewPrivileges=yes
__END__

sudo cp kaseya.service /etc/systemd/system/
sudo systemctl --system daemon-reload
sudo systemctl start kaseya.service
