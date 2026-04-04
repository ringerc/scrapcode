#!/bin/bash
set -e -u -o pipefail
superuser_psql=(psql -qAtX -U postgres)
sudo_cmd=(sudo)

if DATADIR="$("${superuser_psql[@]}" -c 'SHOW data_directory')"
then
  echo "data directory: ${DATADIR}"
else
  ecode=$?
  echo "Could not get datadir - failed to connect as user postgres? psql exited with $?"
  exit 1
fi
if ! "${sudo_cmd[@]}" ls -l "${DATADIR}/global/pg_control"; then
  echo 1>&2 "refusing to proceed: datadir not found"
  exit 1
fi

"${sudo_cmd[@]}" chown root "${DATADIR}/global/pg_control"
# this should fail
if "${superuser_psql[@]}" -c 'CHECKPOINT;'
then
  echo 1>&2 "CHECKPOINT unexpectedly succeded"
  exit 2
elif [ $? -eq 2 ]
then
  echo 1>&2 "psql exited with error 2 as expected"
else
  echo 1>&2 "Unexpected exit from psql: $?"
  exit 2
fi

echo 1>&2 "To undo:"
printf 1>&2 '  %s chown %q %q\n' "${sudo_cmd[*]}" "--reference=${DATADIR}/global/pg_filenode.map" "${DATADIR}/global/pg_control"
printf 1>&2 '  %s chmod %q %q\n' "${sudo_cmd[*]}" "--reference=${DATADIR}/global/pg_filenode.map" "${DATADIR}/global/pg_control"
echo 1>&2 "  # or whatever the appropriate restart command on your system is:"
echo 1>&2 "  sudo systemctl restart postgresql"
