#!/bin/bash
#
# Deploy a promtorture with the supplied arguments, let it run, and grab some
# prometheus metrics + a tsdb snapshot and a metrics dump from it.
#

set -e -u -o pipefail

source scripts/config

if ! "${kubectl[@]}" delete deployment promtorture; then
  echo 1>&2 "Failed to delete promtorture deployment, presumed nonexistent"
fi

# Nuke the prometheus for a clean slate
./scripts/promnuke

# Deploy the promtorture job
./scripts/kind-deploy.sh "$@"

# Wait for some scrapes
sleep 60

# Grab metrics, snapshot etc
./scripts/grab-metrics.sh "$@"

# vim: set ts=2 sw=2 et ai :
