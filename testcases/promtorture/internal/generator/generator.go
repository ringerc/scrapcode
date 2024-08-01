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
	targetNumber int
}

func (t target) targetIndexes(m metrics) []int {
	indexes := make([]int, len(m.cfg.Targets))
	remainder := t.targetNumber
	for i := 0; i < len(m.cfg.Targets); i++ {
		indexes[i] = remainder % m.cfg.Targets[i]
		remainder = remainder / m.cfg.Targets[i]
	}
	return indexes
}

// string representation of the target labels
func (t *target) indexesString(m metrics) string {
	s := ""
	for i, v := range t.targetIndexes(m) {
		if i > 0 {
			s += ","
		}
		s += strconv.Itoa(v)
	}
	return s
}

func (t target) targetLabels(m metrics) []string {
	indexes := t.targetIndexes(m)
	labels := make([]string, len(indexes))
	for i, v := range indexes {
		labels[i] = strconv.Itoa(v)
	}
	return labels
}

func (t target) setValues(m metrics, gaugeValue float64) {
	if m.cfg.InfoMetricsLabels > 0 {
		labels := t.infoLabels(m)
		m.infoMetric.WithLabelValues(labels...).Set(1)
	}
	for i := 0; i < m.cfg.GaugeMetrics; i++ {
		labels := t.targetLabels(m)
		m.gaugeMetrics[i].WithLabelValues(labels...).Set(gaugeValue)
	}
}

func (t target) infoLabels(m metrics) []string {
	labels := t.targetLabels(m)
	for i := 0; i < m.cfg.InfoMetricsLabels; i++ {
		labels = append(labels, fmt.Sprintf("t_%d_i_%d", t.targetNumber, i))
	}
	return labels
}

type metrics struct {
	cfg           Config
	tortureMetric *prometheus.GaugeVec
	infoMetric    *prometheus.GaugeVec
	gaugeMetrics  []*prometheus.GaugeVec
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

	targetLabelNames := make([]string, len(cfg.Targets))
	infoMetricLabelNames := make([]string, len(targetLabelNames)+cfg.InfoMetricsLabels)
	for i, _ := range cfg.Targets {
		targetLabelNames[i] = fmt.Sprintf("target_label_%d", i)
		infoMetricLabelNames[i] = targetLabelNames[i]
	}
	for i := len(cfg.Targets); i < (cfg.InfoMetricsLabels + len(cfg.Targets)); i++ {
		infoMetricLabelNames[i] = fmt.Sprintf("info_label_%d", i)
	}

	metrics := metrics{
		cfg: cfg,
		infoMetric: prometheus.NewGaugeVec(prometheus.GaugeOpts{
			Name: "target_info",
			Help: "info metric for each target, with wider label set",
		}, infoMetricLabelNames),
		tortureMetric: prometheus.NewGaugeVec(prometheus.GaugeOpts{
			Name: "promtorture_info",
			Help: "Info on promtorture's current configuration and arguments",
		}, []string{"targets", "info_labels", "gauge_metrics"}),
		gaugeMetrics: make([]*prometheus.GaugeVec, cfg.GaugeMetrics),
	}
	for i := 0; i < cfg.GaugeMetrics; i++ {
		metrics.gaugeMetrics[i] = prometheus.NewGaugeVec(prometheus.GaugeOpts{
			Name: fmt.Sprintf("gauge_metric_%d", i),
			Help: fmt.Sprintf("Gauge metric %d", i),
		}, targetLabelNames)
	}
	metrics.tortureMetric.WithLabelValues(
		fmt.Sprintf("%v", cfg.Targets),
		fmt.Sprintf("%d", cfg.InfoMetricsLabels),
		fmt.Sprintf("%d", cfg.GaugeMetrics),
	).Set(1)

	targets := make([]target, totalTargets)
	for targetNumber := 0; targetNumber < totalTargets; targetNumber++ {
		// Yeah, this is pointless memory use for now to duplicate the
		// array index as a field in each struct. But it'll make it
		// easier when I want to give the targets some more brains
		// later.
		t := targets[targetNumber]
		t.targetNumber = targetNumber
		t.setValues(metrics, float64(t.targetNumber))
	}

	reg.MustRegister(metrics.tortureMetric)
	if cfg.InfoMetricsLabels > 0 {
		reg.MustRegister(metrics.infoMetric)
	}
	for i := 0; i < cfg.GaugeMetrics; i++ {
		reg.MustRegister(metrics.gaugeMetrics[i])
	}

	return reg
}
