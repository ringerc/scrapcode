#!/bin/bash

set -e -u -o pipefail -vx

source scripts/config

if ! grep -q "${kind_cluster_name}" <<< $(kind get clusters); then
  kind create cluster --name "${kind_cluster_name}"
fi

# Kustomize lacks support for caching remote bases
# per https://github.com/kubernetes-sigs/kustomize/issues/1431
mkdir -p .cache/kube-prometheus
if [ ! -f .cache/kube-prometheus/kube-prometheus.yaml ]; then
  kustomize build https://github.com/prometheus-operator/kube-prometheus -o .cache/kube-prometheus/kube-prometheus.yaml
fi
if [ -f .cache/kube-prometheus/kustomization.yaml ]; then
  rm .cache/kube-prometheus/kustomization.yaml
fi
(cd .cache/kube-prometheus && kustomize create && kustomize edit add resource kube-prometheus.yaml)

# Deploy it, and stop kapp adding its app labels to work around
# https://github.com/carvel-dev/kapp/issues/381
kustomize build kubernetes/kube-prometheus \
  | kapp deploy -a kube-prometheus -y \
      --default-label-scoping-rules=false \
      --apply-default-update-strategy=fallback-on-replace \
      -f -

# vim: et ts=2 sw=2 sts=2 ft=bash ai
