#!/bin/bash
#
# Deploy a promtorture with the supplied arguments, let it run, and grab some
# prometheus metrics + a tsdb snapshot and a metrics dump from it.
#
# Args are passed through to promtorture directly. See:
#    go build
#    ./promtorture --help
# for available options.

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
# (should really hit the prom api for this)
sleep 60

# Grab metrics, snapshot etc
./scripts/grab-metrics.sh "$@"

# vim: set ts=2 sw=2 et ai :
