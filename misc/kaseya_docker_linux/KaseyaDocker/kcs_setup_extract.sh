#!/bin/bash

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

# Expected files found?
