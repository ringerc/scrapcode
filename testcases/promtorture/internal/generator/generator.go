/*
 * Generate varied prometheus metrics
 */
package generator

import (
	"fmt"
	"log"
	"strconv"

	"github.com/prometheus/client_golang/prometheus"
)

type Config struct {
	Port              int
	Targets           []int
	InfoMetricsLabels int
	GaugeMetrics      int
}

type target struct {
	targetNumber  int
	targetIndexes []int
	targetLabels  prometheus.Labels
	infoMetric    *prometheus.Gauge
	gaugeMetrics  []prometheus.Gauge
}

// string representation of the target labels
func (t *target) indexesString() string {
	s := ""
	for i, v := range t.targetIndexes {
		if i > 0 {
			s += ","
		}
		s += strconv.Itoa(v)
	}
	return s
}

func createTarget(cfg Config, targetNumber int, targetIndexes []int) target {
	t := target{
		targetNumber:  targetNumber,
		targetIndexes: targetIndexes,
	}
	t.targetLabels = prometheus.Labels{}
	for i, v := range targetIndexes {
		t.targetLabels["target_label_"+strconv.Itoa(i)] = strconv.Itoa(v)
	}

	log.Printf("Creating target %d with indexes %v, labels %v", targetNumber, targetIndexes, t.targetLabels)

	indexesStr := t.indexesString()

	if cfg.InfoMetricsLabels > 0 {
		infoMetricLabels := prometheus.Labels{}
		// copy the target labels to the info metric label set
		for k, v := range t.targetLabels {
			infoMetricLabels[k] = v
		}
		// add info labels. The names must be consistent for each
		// target, and the values should differ since we presume the
		// targets are unique. So the target indexes will be used for
		// the info-metric values.
		for i := 0; i < cfg.InfoMetricsLabels; i++ {
			infoMetricLabels["info_label_"+strconv.Itoa(i)] = indexesStr
		}
		infoMetric := prometheus.NewGauge(prometheus.GaugeOpts{
			Name:        "info_metric",
			Help:        "Info metric",
			ConstLabels: infoMetricLabels,
		})
		t.infoMetric = &infoMetric
		//log.Printf("Created info-metric %s", (*t.infoMetric).Desc().String())
	}

	t.gaugeMetrics = make([]prometheus.Gauge, cfg.GaugeMetrics)
	for i := 0; i < cfg.GaugeMetrics; i++ {
		t.gaugeMetrics[i] = prometheus.NewGauge(prometheus.GaugeOpts{
			Name:        fmt.Sprintf("gauge_metric_%d", i),
			Help:        fmt.Sprintf("Gauge metric %d", i),
			ConstLabels: t.targetLabels,
		})
		//log.Printf("Created gauge metric %s", t.gaugeMetrics[i].Desc().String())
	}

	return t
}

func (t *target) register(reg *prometheus.Registry) {
	if t.infoMetric != nil {
		reg.MustRegister(*t.infoMetric)
	}
	for _, m := range t.gaugeMetrics {
		reg.MustRegister(m)
	}
}

func CreateRegistry(cfg Config) *prometheus.Registry {

	// The number of targets to generate is the array product of
	// cfg.Targets. The number of labels can vary, so we'll use a counter
	// array, incrementing each value until it reaches the limit then reset
	// to 0 and roll over to increment the next counter. Dumb, but easy.
	totalTargets := 1
	targetCounters := make([]int, len(cfg.Targets))
	for i := 0; i < len(cfg.Targets); i++ {
		targetCounters[i] = 0
		totalTargets *= cfg.Targets[i]
	}

	log.Printf("creating registry for %d targets: %v", totalTargets, cfg.Targets)

	reg := prometheus.NewRegistry()

	for targetNumber := 0; targetNumber < totalTargets; targetNumber++ {
		remainder := targetNumber
		for i := 0; i < len(targetCounters); i++ {
			targetCounters[i] = remainder % cfg.Targets[i]
			remainder = remainder / cfg.Targets[i]
		}

		t := createTarget(cfg, targetNumber, targetCounters)
		t.register(reg)
	}

	return reg
}
