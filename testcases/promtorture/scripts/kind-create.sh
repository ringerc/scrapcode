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

# this'll re-apply the CRDs but that's harmless
#
# ServiceMonitors are omitted deliberately because for this test we only want to see
# metrics for explicitly named targets.
#
yq '.|select((.kind != "CustomResourceDefinition") and (.kind != "ServiceMonitor"))' |  "${kubectl[@]}" apply --server-side -f -
"${kubectl[@]}" apply --server-side -f .cache/kube-prometheus.yaml

# Scale down to 1 replica and don't deploy alertmanager
"${kubectl[@]}" patch -n monitoring prometheus k8s --type merge --patch-file /dev/stdin <<'__END__'
apiVersion: monitoring.coreos.com/v1
kind: Prometheus
spec:
  alerting:
    alertmanagers: []
  replicas: 1
  containers: # this is strategically merged by the operator with the default spec, see kubectl explain prometheus.spec.containers
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
      env:
        # Set the GOMEMLIMIT to a high proportion of the container's memory
        # limit, but not equal to it, so there's room for other processes,
        # runtime overhead, error margins in different usage computation
        # methods etc. We can refine this over time. I'm starting with 95%
        # of the pod RAM limit since we also have a config reloader container
        # etc. See https://pkg.go.dev/runtime
        - name: GOMEMLIMIT
          value: 450MiB
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
