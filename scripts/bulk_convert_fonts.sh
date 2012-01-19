#!/bin/bash
#
# Simple script to batch-convert directories containing large numbers
# of fonts from OpenType to TrueType format
#
# This script is released into the public domain. I grant no warranty
# and will accept no liability of any kind for any use of this script. 
#
set -e -u

# Number of parallel conversion threads to run
#
# There's a significant memory cost to each and too many will cause your
# system to bog down thrashing on swap, so be conservative unless you have
# oodlegigglebytes of RAM. In particular, if you're simplifying cubic
# outlines to quadratic as part of conversion to type 1 or to otf,
# you're going to really hurt for RAM with more than one conversion job.
#
declare -i CPUS=1

# Max number of font files each fontforge instance may process
# before it is terminated and another is launched. This is necessary
# because fontforge appears to leak a lot of memory even when fonts
# are Close()d properly. You may need to tweak this value or reduce the
# number of parallel conversion runs ($CPUS) if you find your
# system starts running out of memory while converting fonts.
#
declare -i FFNFILES=10

# Output format, one of .pfb .ttf .otf .svg
OUTFORMAT=${OUTFORMAT:-.ttf}

if test $# -lt 1; then
  echo
  echo "$0: Insufficient arguments"
  echo
  echo "$0: Converts otf fonts to ttf fonts recursively within a directory"
  echo "    Output fonts are written with a different ext to the same place"
  echo
  echo "Usage: $0 dir-to-convert [dir2 [dir3 [...]]]"
  echo
  exit 1
fi

# Write the FontForge script out
cat > convert.pe <<__END__
#!/usr/bin/env fontforge
i=1
format="$OUTFORMAT"
while ( i<\$argc )
  Open(\$argv[i])
  Print("---FONT---: ", \$curfont)
  SelectAll()
  # Convert cubic splines to quadratic if this font has cubic
  # splines (order=2) and we're converting to a format that
  # doesn't allow them:
  if ( \$order==2 && (format==".otf" || format==".pfb" ))
    Print("Simplifying cubic glyphs to quadratic for otf/pfb output")
    SetFontOrder(3)
    Simplify(128+32+8,1.5)
  endif
  # Re-scale metrics to be appropriate for the font format
  if (format==".otf" || format==".pfb")
    ScaleToEm(1000)
  else
    ScaleToEm(2048)
    RoundToInt()
  endif   
  # PostScript Type 1 fonts are limited to 1-byte encodings,
  # so re-encode 2-byte encoding fonts to latin-9 (iso-8859-9)
  # which is latin-1 with euro. If you need a different encoding,
  # change it here.
  # 
  # You could instead split the font into multiple PFB fonts
  # but you'd then need to be able to tell your app which font
  # to use for which glyph. See "*%s*.pf[ab]" in the FontForge
  # documentation for Generate(...):
  #
  #  http://fontforge.sourceforge.net/scripting-alpha.html#G  
  #
  #
  #
  if (format==".pfb" && (\$iscid || SizeOf(\$selection) > 255) )
    Print("Re-encoding big or CID font to latin-9")
    Reencode("iso-8859-9")
  endif
  Generate(\$argv[i]:r + format)
  Close()
  i = i+1
endloop
__END__

# Write the log filter awk script out
cat > logfilter.awk <<__END__
BEGIN {
  skip=1;
}

/^[^ ]/ { skip=1; }
/^The 'size' feature of this font conforms to Adobe's early misinterpretation of the otf standard/ { skip=0; }
/^Warning: Mac and Windows entries in the 'name' table/ { skip=0; }
/^Warning: Mac string is a subset of the Windows string/ { skip=0; }
/^The glyph named (mu|Delta|Omega|one.superior|two.superior|three.superior|enspace) is mapped to/ { skip=0; }
/^The following table\(s\) in the font have been ignored/ { skip=0; }

/.*/ {
  if (skip==1) { print \$0; }
}
__END__

chmod a+x ./convert.pe
find "$@" -type f -name \*.otf -print0 |\
        xargs -P $CPUS -n $FFNFILES -0 ./convert.pe 2>&1 | awk -f logfilter.awk | tee log

rm convert.pe logfilter.awk
