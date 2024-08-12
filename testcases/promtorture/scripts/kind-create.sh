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

kustomize build kubernetes/kube-prometheus | kapp deploy -a kube-prometheus -f - -y

# vim: et ts=2 sw=2 sts=2 ft=bash ai
