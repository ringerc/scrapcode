#!/bin/bash

KILL_BACKEND_CMD=(sudo kill -SIGSEGV)

coproc PSQL { psql -qAtX  "dbname=postgres keepalives_idle=1 keepalives_interval=1 keepalives_count=5 options='-c idle_session_timeout=5s'"  ; }

trap 'ecode=$?; if [ -n "${PSQL_PID}" ]; then kill -TERM "${PSQL_PID}"; fi; echo "Unexpected error $ecode at $BASH_LINENO"; exit 1;' ERR

echo >&${PSQL[1]} 'SELECT pg_backend_pid();'
read -r -u "${PSQL[0]}" -r BACKEND_PID
echo >&${PSQL[1]} 'SELECT pg_postmaster_start_time();'
read -r -u "${PSQL[0]}" POSTMASTER_START_TIME
echo >&${PSQL[1]} 'SELECT pg_sleep(10);'
POSTMASTER_PID="$(ps -p "${BACKEND_PID}" --no-header -o ppid:1)"

echo 1>&2 "Postmaster PID: ${POSTMASTER_PID}"
echo 1>&2 "Backend PID: ${BACKEND_PID}"
echo 1>&2 "Postmaster start time: ${POSTMASTER_START_TIME}"
echo 1>&2 "Backend:"
ps -p "${BACKEND_PID}" -o pid=,cmd= 1>&2

echo 1>&2 "Sending kill signal:"
if "${KILL_BACKEND_CMD[@]}" "${BACKEND_PID}"; then
  echo 1>&2 "sent SIGKILL to backend pid ${BACKEND_PID}"
else
  ecode=$?
  echo 1>&2 "kill -KILL ${PID} failed: $ecode"
  exit 1
fi
echo 1>&2 "waiting for psql ${PSQL_PID} to exit"
if wait "${PSQL_PID}"; then
	echo 1>&2 "psql exited with 0, something must've gone wrong"
	exit 1
else
	ecode=$?
	if [ "${ecode}" -eq 2 ]; then
		echo 1>&2 "psql exited with 2 as expected"
	else
		echo 1>&2 "psql exited with unexpected error ${ecode} instead of 2"
	fi
fi

echo 1>&2 "Waiting for postmaster to restart"
while ! NEW_POSTMASTER_START_TIME="$(psql -qAtX -c 'select pg_postmaster_start_time();')"
do
	echo 1>&2 "Waiting for postmaster to restart"
	sleep 1
done
echo 1>&2 "Postmaster start time after kill: ${NEW_POSTMASTER_START_TIME}"
if [ "${POSTMASTER_START_TIME}" = "${NEW_POSTMASTER_START_TIME}" ]; then
	echo 1>&2 "Postmaster start time did not change, something went wrong"
	exit 1
fi
