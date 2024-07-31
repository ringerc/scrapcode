#!/bin/bash

set -e -u -o pipefail -x

source scripts/config

if ! grep -q "${kind_cluster_name}" <<< $(kind get clusters); then
  kind create cluster --name "${kind_cluster_name}"
fi

mkdir -p .cache
if [ ! -f .cache/kube-prometheus.yaml ]; then
  kustomize build https://github.com/prometheus-operator/kube-prometheus -o .cache/kube-prometheus.yaml
fi

# because kubectl apply still doesn't know how to wait for CRDs
# before applying the rest...
yq '.|select(.kind == "CustomResourceDefinition")' .cache/kube-prometheus.yaml | "${kubectl[@]}" apply --server-side -f -

"${kubectl[@]}" wait --for=condition=established --timeout=60s crd --all

yq '
  .
  |select(
    (.kind != "CustomResourceDefinition")
    and (.kind != "AlertManager")
   )' |  "${kubectl[@]}" apply --server-side -f -
"${kubectl[@]}" apply --server-side -f .cache/kube-prometheus.yaml

# Scale down to 1 replica and don't deploy alertmanager
"${kubectl[@]}" patch -n monitoring prometheus k8s --type merge --patch-file /dev/stdin <<'__END__'
apiVersion: monitoring.coreos.com/v1
kind: Prometheus
spec:
  alerting:
    alertmanagers: []
  replicas: 1
  # args to pass to the prometheus container, see kubectl explain prometheus.spec.additionalArgs
  additionalArgs:
  - name: web.enable-admin-api
  - name: web.enable-remote-write-receiver
  # https://prometheus.io/docs/prometheus/latest/feature_flags/#memory-snapshot-on-shutdown
  # pointless when we aren't using a PV, but should move to it
  # for BA anyway
  - name: enable-feature
    value: memory-snapshot-on-shutdown
  # https://prometheus.io/docs/prometheus/latest/feature_flags/#extra-scrape-metrics
  - name: enable-feature
    value: extra-scrape-metrics
  # https://prometheus.io/docs/prometheus/latest/feature_flags/#per-step-stats
  - name: enable-feature
    value: promql-per-step-stats
  # https://prometheus.io/docs/prometheus/latest/feature_flags/#auto-gomemlimit
  - name: enable-feature
    value: auto-gomemlimit
  - name: auto-gomemlimit.ratio
    value: "0.9"
  # https://prometheus.io/docs/prometheus/latest/feature_flags/#auto-gomaxprocs
  - name: enable-feature
    value: auto-gomaxprocs
  # https://prometheus.io/docs/prometheus/latest/feature_flags/#created-timestamps-zero-injection
  - name: enable-feature
    value: created-timestamp-zero-ingestion
  # this is strategically merged by the operator with the default spec, see kubectl explain prometheus.spec.containers
  containers:
    - name: config-reloader
      securityContext:
        runAsNonRoot: true
        runAsUser: 1000
        allowPrivilegeEscalation: false
        privileged: false
        readOnlyRootFilesystem: true
        capabilities:
          drop:
            - ALL
    - name: prometheus
      securityContext:
        runAsNonRoot: true
        runAsUser: 1000
        allowPrivilegeEscalation: false
        privileged: false
        readOnlyRootFilesystem: true
        capabilities:
          drop:
            - ALL
  resources:
    limits:
      cpu: 1000m
      memory: 500Mi
    requests:
      cpu: 200m
      memory: 500Mi
__END__


"${kubectl[@]}" wait --for=condition=available --timeout=300s deployment --all -n monitoring
"${kubectl[@]}" wait --for=condition=ready --timeout=300s pod --all -n monitoring

# vim: et ts=2 sw=2 sts=2 ft=bash ai
