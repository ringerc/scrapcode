/*
 * hacked up version of the victoria metrics prom exporter demo, for generating
 * massive cardinality and throwing it at prom.
 */
package main

import (
	"fmt"
	"strings"
	"time"
	"net/http"
	"log"
	"math/rand"
	"github.com/google/uuid"
	"github.com/VictoriaMetrics/metrics"
)

func addMetric(basename string, ndims int) {
	var labels []string = []string{
		"fixed1=\"constant\"",
		"fixed2=\"very very very very very very very very very long long long long long long value value value value\"",
	}
	for i := 0; i < ndims; i++ {
		dim := fmt.Sprintf("dim%d=\"%s\"", i, uuid.New().String())
		labels = append(labels, dim)
	}
	metric_name := fmt.Sprintf("%s{%s}", basename, strings.Join(labels,","))
	// Discard the metric, we're just making noise here. The metric value will be 0.
	// TODO: remember them a while then age them out?
	metric_func := func() float64 { return rand.Float64() }
	_ = metrics.NewGauge(metric_name, metric_func)
}

func metricsGenerator() {
	for {
		addMetric("basic", 1)
		addMetric("wide", 10)
		addMetric("extreme", 40)

		time.Sleep(1 * time.Second)
	}
}

func main() {
	// front-load a massive pile of metrics
	for i := 0; i < 400000; i++ {
		metric_name := fmt.Sprintf("fixed%d", i)
		metric_func := func() float64 { return rand.Float64() }
		_ = metrics.NewGauge(metric_name, metric_func)
	}
	// and add more with wide dims regularly
	go metricsGenerator();
	http.HandleFunc("/metrics", func(w http.ResponseWriter, req *http.Request) {
		metrics.WritePrometheus(w, false)
	})
	log.Fatal(http.ListenAndServe(":8080", nil))
}
