#!/bin/bash
#
# Build a promtorture and deploy it. All arguments are passed verbatim to the
# promtorture binary in the kube deployment.
#
# See:
#     go build
#     ./promtorture --help
#
set -e -u -o pipefail

source scripts/config

echo 1>&2 "Deploying promtorture with arguments: $@"

tmpdir="$(mktemp -d -t -p . promtorture-tmp-XXXXXX)"
trap 'rm -rf "$tmpdir"' EXIT

docker buildx build -t promtorture .

kind load docker-image promtorture --name promtorture

# kustomize SIG remain hostile to templating or
# substitutions (https://kubectl.docs.kubernetes.io/faq/kustomize/eschewedfeatures/)
# and I don't want to dirty the versioned tree with a generated patch, so we
# need a generated overlay. (envsubst is a menace and I don't want to add a
# whole separate tool like ytt here.)
cat > "$tmpdir/kustomization.yaml" <<'__END__'
apiVersion: kustomize.config.k8s.io/v1beta1
kind: Kustomization
metadata:
  annotations:
    config.kubernetes.io/local-config: "true"
resources:
  - ../kubernetes/promtorture
patches:
  - target:
      kind: Deployment
      name: promtorture
      namespace: default
    path: patch-promtorture-deployment.yaml
__END__

# Templating the args in is a bit ugly
ARGS_JSON="$(jq -c --null-input '$ARGS.positional' --args -- "$@")"
ARGS_JSON="${ARGS_JSON}" ARGS="$*" yq --prettyPrint '
  .spec.template.spec.containers[0].args=env(ARGS_JSON)
  | .metadata.annotations["promtorture/arguments"]=strenv(ARGS)
  | .spec.template.metadata.annotations["promtorture/arguments"]=strenv(ARGS)
  ' \
  > "${tmpdir}/patch-promtorture-deployment.yaml" \
  <<__END__
apiVersion: apps/v1
kind: Deployment
metadata:
  name: promtorture
  namespace: default
  annotations:
    promtorture/arguments: ""
spec:
  template:
    spec:
      metadata:
        annotations:
          promtorture/arguments: ""
      containers:
        - name: promtorture
__END__

kustomize build "${tmpdir}" \
  | tee /dev/stderr \
  | kapp deploy -a promtorture -f - -y \
      --default-label-scoping-rules=false \
      --apply-default-update-strategy=fallback-on-replace \
      --diff-changes

echo 1>&2 "Promtorture is running on service 'promtorture' on port 'metrics' (TCP/8080)"

# vim: et ts=2 sw=2 sts=2 ft=bash ai
