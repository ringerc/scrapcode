# promtorture

Make prometheus feel really bad. For science.

## quickstart

```
./scripts/kind-create.sh
./scripts/kind-deploy.sh

sleep 10

./scripts/resources
./scripts/promapi -m tsdb | yq .data.headStats
./scripts/promapi -m tsdb -l 100

```

## promtorture

for args:

```
go build
./promtorture --help
```

## scripts

- [`kind-create.sh`](./scripts/kind-create.sh): Create a kind cluster and deply kube-prometheus
- [`kind-deploy.sh`](./scripts/kind-deploy.sh): Deploy promtorture, takes CLI options for target and label counts
- [`promapi`](./scripts/promapi): A simple prometheus API client, see
  `./scripts/promapi -h`. Can get TSDB info, make snapshots, etc.
- [`promcmd`](./scripts/promcmd): wrapper for running shell commands on the prometheus pod. Useful for running
  `promtool` etc. The script source contains some notes on handy commands.
- [`promlogs`](./scripts/promlogs): tail logs from the prometheus pod
- [`promnuke`](./scripts/promnuke): delete all prometheus data
- [`promtorture-args`](./scripts/promtorture-args): report the container args for the running promtorture instance
- [`resources`](./scripts/resources): dump the resources for the prometheus pod
- [`socks5`](./scripts/socks5): start a socks5 proxy to the prometheus pod. You can use this with
  the `http_proxy=socks5://localhost:1081` env-var to proxy requests to the kube cluster, so you can
  use in-cluster URIs like `http://prometheus-k8s.monitoring.svc.cluster.local:9090` from commands
  like `curl` and `promtool`. See also the `curl` option `--socks5-hostname`.
- [`get-promtool`](./scripts/get-promtool): download the `promtool` binary from a prometheus release,
  for if you want to run `promtool` over SOCKS5 or port-forward rather than in the prom container, e.g
  for `promtool debug` dump generation.
