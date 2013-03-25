#!/bin/bash
filetoread=$1
sep=$(printf '%04x%04x\n' $RANDOM $RANDOM)
cat <<__END__
INSERT INTO testfile(filecontent) VALUES (
\$x${sep}\$$(cat ${filetoread})\$x${sep}\$
);
__END__
