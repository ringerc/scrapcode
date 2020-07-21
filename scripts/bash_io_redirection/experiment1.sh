#!/bin/bash

# Open the log file we want to write to as FD 7
exec {LOGFD}>/tmp/test.log

# Open a new FD 8 that copies its input to FDs 7 and 1 (log file and stdout).
# By using the FD instead of directly writing to the file, we help tee cope
# with file position issues.  Hopefully.
exec {DUPFD}> >(tee /dev/fd/${LOGFD} )

# Send echo to fd1 (stdout). Result will only be on stdout.
echo 'testing fd1'

# Send echo to FD8. Result is going to be on on the log and on stdout. Hopefully.
echo 'testing fd7' >&${DUPFD}

# YESSSS

# Now what about the other way? We want to copy stdout and stderr to a log file
# along with script trace info, but also emit stdout to the terminal.
#
# So we need to open a log file:
exec {LOGFD2}>/tmp/test2.log

# copy stdout to the logfile via tee, retaining stdout too
exec {STDOUTCOPY}> >(tee /dev/fd/${LOGFD2})

