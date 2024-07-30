/*
 * prometheus metrics server endpoint
 */
package server

import (
	"log"
	"net/http"

	"github.com/prometheus/client_golang/prometheus"
	"github.com/prometheus/client_golang/prometheus/promhttp"
)

func ServeForever(reg *prometheus.Registry) {
	http.Handle("/metrics", promhttp.HandlerFor(reg, promhttp.HandlerOpts{Registry: reg}))
	log.Printf("Listening on :8080 for /metrics")
	log.Fatal(http.ListenAndServe(":8080", nil))
}
