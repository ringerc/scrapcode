# promtorture

Make prometheus feel really bad. For science.

## prerequisites

* [`go`](https://golang.org/)
* [`docker-ce`](https://docs.docker.com/engine/install/)
* [`kind`](https://kind.sigs.k8s.io/)
* [`kapp`](https://get-kapp.io/)
* [`kustomize`](https://kustomize.io/)
* [`kubectl`](https://kubernetes.io/docs/tasks/tools/)
* [`jq`](https://stedolan.github.io/jq/)
* [`yq`](https://mikefarah.gitbook.io/yq/)

## quickstart

```
./scripts/kind-create.sh
./scripts/kind-deploy.sh

sleep 10

./scripts/resources
./scripts/promapi -m tsdb-head
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
- [`clean-collect-promtorture`](./scripts/clean-collect-promtorture): Delete prometheus data, deploy promtorture with specified args, and collect various prom stats and metrics using `script/grab-metrics.sh`
- [`scripts/grab-metrics.sh`](./scripts/grab-metrics.sh): Grab various data about prometheus's memory use, storage, the current promtorture workload scrapes, etc. Collects a tsdb snapshot.
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

## Handy tips

### Use `k8s-insider` to talk to the services

(No OSX support at time of writing)

See https://github.com/TrueGoric/k8s-insider

Assuming Ubuntu 24.04:

```
sudo apt install wireguard
kubectl insider install --pod-cidr 10.244.0.0/16
kubectl insider create network
kubectl insider connect
resolvectl domain insider0 cluster.local
```

[kind docs on pod subnet addressing](https://kind.sigs.k8s.io/docs/user/configuration/#pod-subnet)

This will give you the ability to directly query kube services like `prometheus-k8s.monitoring.svc.cluster.local` or visit http://grafana.monitoring.svc.cluster.local:3000 directly in a browser.

### Use `socks5` to talk to the services

```
./scripts/socks5
```

Then set your `http_proxy` to `socks5://localhost:1081` and you can use `curl`
etc to talk to the services.

### Grafana dashboard

See [`resources`](./resources/grafana-dashboards/promtorture.json).

There's no script to auto-load it yet, import it yourself.

### Use `promtool` to run promql, inspect labels etc

e.g.

```
scripts/promcmd promtool query instant http://localhost:9090 'up{job="monitoring/promtorture"}'
```

Other tips in `scripts/promcmd` comments.

# Notes

The apparent inability to force Prometheus to eagerly checkpoint its HEAD or to
force a compaction is a bit of a pain; we can't easily analyse behaviour with
querying data back from TSDB without waiting a couple of hours. We can force a
snapshot, but that doesn't actually flush the HEAD.

# See also

* [prometheus docs on storage](https://github.com/juliusv/prometheus-docs/blob/master/content/docs/operating/storage.md)
