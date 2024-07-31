#!/bin/bash

kind_cluster_name=promtorture

if ! grep -q "${kind_cluster_name} " <<< $(kind get clusters); then
  kind create cluster --name "${kind_cluster_name}"
fi

mkdir -p .cache
if [ ! -f .cache/kube-prometheus.yaml ]; then
  kustomize build https://github.com/prometheus-operator/kube-prometheus -o .cache/kube-prometheus.yaml
fi

# because kubectl apply still doesn't know how to wait for CRDs
# before applying the rest...
yq '[.items[] | select(.kind == "CustomResourceDefinition")]' .cache/kube-prometheus.yaml | kubectl apply -f -

# Sleeps suck, but we need to wait for the CRDs to be created and don't want to
# overcomplicate this script by looping through kubectl api-resources and checking
# for the CRDs we need.
sleep 1

# this'll re-apply the CRDs but that's harmless
kubectl apply -f .cache/kube-prometheus.yaml

# vim: et ts=2 sw=2 sts=2 ft=bash ai
