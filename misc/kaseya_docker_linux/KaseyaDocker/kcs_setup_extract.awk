#!/usr/bin/gawk --characters-as-bytes

/^# {{{/ {
	extract_setup = 1
	printf "Extracting KcsSetup...."
	next
}
extract_setup == 1 {
	print | "base64 -d > KcsSetup"
}
/^# }}}/ {
	extract_setup = 0
	fflush("base64 -d > KcsSetup")
	close("base64 -d > KcsSetup")
	system("7z x KcsSetup")
	printf "done\n"
	next
}

/^# Starting FileExtractorItem:/ {
	if (itemheader) {
		printf("error: got %s when processing existing item", $0)
		exit
	}
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
	size=$2
	next
}
itemheader && /^Offset:/ {
	offset=$2
	# the offset for some insane reason points to this line, above the
	# actual embedded binary.
	offset += length("# Starting Embedded Installer Files\r\n")
	# No idea why the size is wrong by 2 bytes either
	size += 2
	next
}
itemheader && /^File:/ {
	next
}
/^# End of FileExtractorItem:/ {
	itemheader = 0
	printf "Type: %s; size: %s\n", filename, size
	if (offset) {
		system("dd if=KcsSetup of='" filename "' bs=1 skip='" offset "' count='" size "'")
	} else {
		itembody = 1
		printf "" > filename
	}
	next
}
itembody {
	print > filename
	next
}
itemheader {
	printf("unrecognised item line %s\n", $0)
	exit
}
