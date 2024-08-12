#!/bin/bash
#
# Grab some prometheus metrics + a tsdb snapshot and a metrics dump from
# the prometheus server running in the kind cluster.
#
# This should be in a real language not bash, but meh.
#

set -e -u -o pipefail -x

source scripts/config

tmpdir="promtorture-metrics-$(date -Isec)"
mkdir "$tmpdir"

echo 1>&2 "Dumping metrics to $tmpdir"

# arguments passed to this script just get dumped to the tempdir, so it's a
# convenient way to record the promtorture invocation
echo "$@" > "$tmpdir/args"

socks5_port=31121
socks5_host=localhost
kubectl socks5-proxy -N socks5-proxy -p 31121 &
socks5_pid=$!
# Becuse socks5-proxy is written in bash this kills the script but not the
# underlying kubectl port-forward process. We should fix this in the
# socks5-proxy script. For now we'll find the child proc and kill it.
# It should also wait with timeout for the wrapper to exit, but bash's wait
# lacks a timeout option...
trap 'kill $(pgrep -P ${socks5_pid}); sleep 10; kill ${socks5_pid}' EXIT
export http_proxy="socks5://${socks5_host}:${socks5_port}"
while : ; do
  # wait for proxy to be ready by checking prometheus is reachable
  if curl -sL http://prometheus-k8s.monitoring.svc.cluster.local:9090 > /dev/null; then
    break
  fi
  sleep 1
done

promtorture_pod="$("${kubectl[@]}" get pod -n default -l app=promtorture -o jsonpath='{.items[0].metadata.name}')"

# metrics queries
mkdir "$tmpdir/metrics"

function instant_query_promtool() {
  echo '# query: ' "$1" > "$tmpdir/$2"
  ./scripts/promcmd promtool query instant http://localhost:9090 "$1" >> "$tmpdir/$2"
}

function instant_query_curl() {
  echo '# query: ' "$1" > "$tmpdir/$2"
  curl -sL -G --data-urlencode "query=$1" "http://prometheus-k8s.monitoring.svc.cluster.local:9090/api/v1/query" | yq --prettyPrint .data >> "$tmpdir/$2"
}

function instant_query() {
  instant_query_curl "$1" "${2}.yaml"
  instant_query_promtool "$1" "${2}.prom"
}


promtorture_label_join=' * on (job,container,pod) group_left(gauge_metrics,info_labels,targets) (promtorture_info{job="monitoring/promtorture",container="promtorture"})'

declare -A queries
queries[prometheus_tsdb_head_series]='increase(prometheus_tsdb_head_series{job="prometheus-k8s",container="prometheus"}[5m])'
queries[prometheus_tsdb_head_chunks]='prometheus_tsdb_head_chunks{job="prometheus-k8s",container="prometheus"}'
queries[delta_prometheus_tsdb_head_chunks]='delta(prometheus_tsdb_head_chunks{job="prometheus-k8s",container="prometheus"}[5m])'
queries[prometheus_tsdb_head_chunks_created_total]='prometheus_tsdb_head_chunks_created_total{job="prometheus-k8s",container="prometheus"}'
queries[prometheus_tsdb_head_chunks_storage_size_bytes]='prometheus_tsdb_head_chunks_storage_size_bytes{job="prometheus-k8s",container="prometheus"}'
queries[prometheus_tsdb_storage_blocks_bytes]='prometheus_tsdb_storage_blocks_bytes{job="prometheus-k8s",container="prometheus"}'
queries[process_resident_memory_bytes]='process_resident_memory_bytes{job="prometheus-k8s",container="prometheus"}'
queries[go_gc_gomemlimit_bytes]='go_gc_gomemlimit_bytes{job="prometheus-k8s",container="prometheus"}'
queries[prometheus_tsdb_symbol_table_size_bytes]='prometheus_tsdb_symbol_table_size_bytes{job="prometheus-k8s",container="prometheus"}'
queries[container_memory_working_set_bytes]='container_memory_working_set_bytes{container="prometheus",pod="prometheus-k8s-0"}'
queries[sum_prometheus_tsdb_head_series]='sum by () (prometheus_target_metadata_cache_bytes{job="prometheus-k8s",container="prometheus"})'
queries[promtorture_max_scrape_duration_seconds]='max(scrape_duration_seconds{job="monitoring/promtorture",container="promtorture",pod="'"${promtorture_pod}"'"})'"${promtorture_label_join}"
queries[promtorture_avg_scrape_duration_seconds]='avg(scrape_duration_seconds{job="monitoring/promtorture",container="promtorture",pod="'"${promtorture_pod}"'"})'"${promtorture_label_join}"
queries[promtorture_scrape_samples_scraped]='scrape_samples_scraped{job="monitoring/promtorture",container="promtorture",pod="'"${promtorture_pod}"'"}'"${promtorture_label_join}"
queries[promtorture_scrape_body_size_bytes]='scrape_body_size_bytes{job="monitoring/promtorture",container="promtorture",pod="'"${promtorture_pod}"'"}'"${promtorture_label_join}"
queries[promtorture_sum_scrape_series_added]='sum(scrape_series_added{job="monitoring/promtorture",container="promtorture",pod="'"${promtorture_pod}"'"})'"${promtorture_label_join}"

# these will be run as instant, max() aggregate and avg() aggregate
declare -A queries_with_aggregates
queries_with_aggregates[go_memstats_heap_sys_bytes]='go_memstats_heap_sys_bytes{job="prometheus-k8s",container="prometheus"}'
queries_with_aggregates[go_memstats_alloc_bytes]='go_memstats_alloc_bytes{job="prometheus-k8s",container="prometheus"}'
queries_with_aggregates[go_memstats_heap_sys_bytes]='go_memstats_heap_sys_bytes{job="prometheus-k8s",container="prometheus"}'
queries_with_aggregates[go_memstats_sys_bytes]='go_memstats_sys_bytes{job="prometheus-k8s",container="prometheus"}'
queries_with_aggregates[go_memstats_heap_inuse_bytes]='go_memstats_heap_inuse_bytes{job="prometheus-k8s",container="prometheus"}'

for query in "${!queries[@]}"; do
  instant_query "${queries[$query]}" "metrics/$query"
done

for query in "${!queries_with_aggregates[@]}"; do
  instant_query "${queries_with_aggregates[$query]}" "metrics/$query"
  instant_query "max(${queries_with_aggregates[$query]})" "metrics/$query-max"
  instant_query "avg(${queries_with_aggregates[$query]})" "metrics/$query-avg"
done

# prom api info dumps
mkdir "$tmpdir/api"
for endpoint in target targetmeta tsdb-head walreplay config flags runtime; do
    ./scripts/promapi -m "${endpoint}" > "$tmpdir/api/${endpoint}.yaml"
done

# collect more than the default 10 labels
./scripts/promapi -m "${endpoint}" -l 200 > "$tmpdir/api/${endpoint}.yaml"

# tsdb snapshot - create, download, get size
mkdir "$tmpdir/tsdb-snapshot"
snap_name="$(./scripts/promapi -m snapshot | yq .data.name)"
echo 1>&2 "Created tsdb snapshot ${snap_name}"
./scripts/promcmd du -ks "/prometheus/snapshots/${snap_name}" > "$tmpdir/tsdb-snapshot/size-kb"
mkdir "$tmpdir/tsdb-snapshot/raw"
"${kubectl[@]}" cp "monitoring/prometheus-k8s-0:/prometheus/snapshots/${snap_name}/" "${tmpdir}/tsdb-snapshot/raw/"

# metrics dump
if [[ "${TSDB_DUMP}" == "true" ]]; then
  # we're dumping the tsdb snapshot here so we don't have to deal with running promtool's own
  # dump and grabbing the file from the container FS.
  #
  # Work around its assumption that there will be WAL for a snapshot:
  ./scripts/promcmd mkdir -p "/prometheus/snapshots/${snap_name}/wal" "/prometheus/replay"
  # and dump to stdout
  compress="lz4"
  if ! type -p lz4 >&/dev/null; then
    echo 1>&2 "no lz4 command, falling back to gzip"
    compress="gzip"
  fi
  ./scripts/promcmd promtool tsdb dump --sandbox-dir-root="/prometheus/replay" "/prometheus/snapshots/${snap_name}" | ${compress} -c > "$tmpdir/tsdb-snapshot/tsdb-dump"."${compress}"
fi

# vim: set ts=2 sw=2 et ai :
