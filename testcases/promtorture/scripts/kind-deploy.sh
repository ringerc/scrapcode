#!/bin/bash

kind_cluster_name=promtorture

targets=1
info_metrics_labels=0
gauge_metrics=1
while getopts "t:i:g:" opt; do
  case ${opt} in
    t )
      targets=$OPTARG
      ;;
    i )
      info_metrics_labels=$OPTARG
      ;;
    g )
      gauge_metrics=$OPTARG
      ;;
    \? )
      echo "Usage: kind-deploy.sh -t <targets> -i <info_metrics_labels> -g <gauge_metrics>"
      echo "e.g."
      echo "  kind-deploy.sh -t 2,2 -i 4 -g 10"
      exit 1
      ;;
  esac
done

docker buildx build -t promtorture .

kind load docker-image promtorture --name promtorture

kubectl=("kubectl", "--context", "kind-${kind_cluster_name}")

"${kubectl[@]}" apply -f /dev/stdin <<__END__
apiVersion: apps/v1
kind: Deployment
metadata:
  name: promtorture
  labels:
    app: promtorture
spec:
  replicas: 1
  selector:
    matchLabels:
      app: promtorture
  template:
    metadata:
      labels:
	app: promtorture
    spec:
      containers:
      - name: promtorture
	image: promtorture
  imagePullPolicy: Never
	ports:
	- containerPort: 8080
    name: metrics
	args:
	- "--port=8080"
  - "--targets=${targets}"
  - "--info-metrics-labels=${info_metrics_labels}"
  - "--gauge-metrics=${gauge_metrics}"
__END__

"${kubectl[@]}" apply -f /dev/stdin <<__END__
apiVersion: v1
kind: Service
metadata:
  name: promtorture
spec:
  selector:
    app: promtorture
  ports:
  - protocol: TCP
    port: 8080
    targetPort: metrics
    name: metrics
__END__

"${kubectl[@]}" apply -f /dev/stdin <<__END__
kind: PodMonitor
apiVersion: monitoring.coreos.com/v1
metadata:
  name: promtorture
  labels:
    app: promtorture
spec:
  selector:
    matchLabels:
      app: promtorture
  namespaceSelector:
    matchNames:
    - default
  podMetricsEndpoints:
  - port: metrics
__END__

"${kubectl[@]}" wait --for=condition=available --timeout=60s deployment/promtorture

echo 1>&2 "Promtorture is running on service 'promtorture' on port 'metrics' (TCP/8080)"

# vim: et ts=2 sw=2 sts=2 ft=bash ai
