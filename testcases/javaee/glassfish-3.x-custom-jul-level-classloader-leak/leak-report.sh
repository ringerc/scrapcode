# Leak detected if OQL query:
# select { leaked: heap.findClass('com.mycompany.leaktest2.TestBean') != null }

# Result is HTML containing string '{ leaked:false, }' or '{ leaked:true, }'. The page kindly puts the OQL
# output on its own line, so we'll just use dirty text processing to extract it.
leaked=$(curl -s -f  'http://localhost:7000/oql/?query=select+\{+leaked%3A+heap.findClass%28%27com.mycompany.leaktest2.TestBean%27%29+%21%3D+null+\}' | grep '{ leaked:\(true\|false\), }' | cut -d : -f 2 | cut -d , -f 1)

if test "$leaked" = "true" ; then
	echo "com.mycompany.leaktest2.TestBean ***WAS*** ***LEAKED***"
	exit 1
elif test "$leaked" = "false" ; then
	echo "No leak of com.mycompany.leaktest2.TestBean was detected"
	exit 0
else
	echo "Unable to determine whether or not a leak occurred. OQL query failure?"
	exit 2
fi
