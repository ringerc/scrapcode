# Prometheus cardinality torture-testing

(This is an earlier version of promtorture, retained here for reference)


Should use https://github.com/prometheus-operator/prometheus-operator/blob/main/Documentation/user-guides/getting-started.md instead for 
setup

```bash
kind create cluster

# should be releasever instead
kustomize build https://github.com/prometheus-operator/kube-prometheus > prom.yaml
kfilt --kind=CustomResourceDefinition < prom.yaml | kubectl create --context kind-kind -f -
kfilt --exclude-kind=CustomResourceDefinition < prom.yaml | kubectl create --context kind-kind -f -
rm prom.yaml

# should be a kustomization
cat > resources-patch.json  <<'__END__'
[
  {
    "op": "add",
    "path": "/spec/resources",
    "value": {
      "requests": {
        "memory": "1Gi",
	"cpu": "4"
      },
      "limits": {
        "memory": "1Gi",
	"cpu": "15"
      }
    }
  },
  {
    "op": "replace",
    "path": "/spec/scrapeInterval",
    "value": "1s"
  },
  {
    "op": "replace",
    "path": "/spec/replicas",
    "value": 1
  }
]
__END__

kubectl patch -n monitoring prometheus/k8s --type json  --patch-file resources-patch.json

docker buildx build -t promkill:latest .
kind load docker-image promkill:latest
kubectl apply --context kind-kind -f config/k8s/deployment.yaml
kubectl apply --context kind-kind -f config/k8s/podmonitor.yaml
```

check exported metrics

```bash
kubectl port-forward $(kubectl get pod -n default -l app=promkill -o name | head -1) 8080:8080
curl -sSLf localhost:8080/metrics
curl -sSLf http://localhost:8080/metrics | wc -l
```

check prom

```bash
kubectl -n monitoring get pods -l app.kubernetes.io/name=prometheus
```

forward prom api

```bash
kubectl -n monitoring port-forward services/prometheus-k8s 8081:8080 9091:9090
```

then visit http://localhost:9091/ for webui or API, e.g. enumerate labels

Make sure it's being scraped

```bash
curl -X GET -sSLf http://localhost:9091/api/v1/targets
```

check metrics dims

```bash
curl -X GET -sSLf http://localhost:9091/api/v1/labels
```

watch mem

```
kube-capacity --pod-labels app.kubernetes.io/name=prometheus -u -c
```


### hacking prom

```
kubectl get -n monitoring prometheus/k8s -oyaml
```

also useful: `GET /api/v1/metadata`, `GET /api/v1/status/runtimeinfo`, `GET /api/v1/status/tsdb`

* `curl -X GET -sSLf http://localhost:9091/api/v1/status/tsdb | jq '.'`
* `curl -X GET -sSLf http://localhost:9091/api/v1/status/runtimeinfo | jq '.'`
* `curl -X GET -sSLf http://localhost:9091/api/v1/status/config | jq -r '.data.yaml'`
* `curl -X GET -sSLf http://localhost:9091/api/v1/status/flags`
* `curl -X GET -sSLf http://localhost:9091/api/v1/metadata| jq '.'`
* `curl -X GET -sSLf http://localhost:9091/flags | jq -r '.data.yaml'`

Watch prom's own metrics

## Poke at prom

```bash
kubectl get -n monitoring prometheus/k8s -oyaml > promcfg.yaml
```
