package cmd

import (
	"fmt"
	"log"
	"os"
	"strconv"
	"strings"

	"github.com/ringerc/scrapcode/testcases/promtorture/internal/generator"
	"github.com/ringerc/scrapcode/testcases/promtorture/internal/server"
	"github.com/spf13/cobra"
)

var rootCmd = &cobra.Command{
	Use:   "promtorture",
	Short: "Generate metrics to make Prometheus sad",
	Long: `Generate varied sets of metrics to test
Prometheus (and compatible) memory, CPU, disk consumption,
computed cardinality etc.

This is intended as a torture test for Prometheus, Thanos,
VictoriaMetrics etc resource use.
`,
	Run: func(cmd *cobra.Command, args []string) {
		var err error
		cfg := generator.Config{}
		cfg.Port, err = cmd.Flags().GetInt("port")
		if err != nil {
			log.Fatalf("while parsing --port: %v", err)
		}
		targetsArg, err := cmd.Flags().GetString("targets")
		if err != nil {
			log.Fatalf("while reading --targets: %v", err)
		}
		cfg.Targets, err = parseTargets(targetsArg)
		if err != nil {
			log.Fatalf("while parsing --targets: %v", err)
		}
		cfg.InfoMetricsLabels, err = cmd.Flags().GetInt("info-metrics-labels")
		if err != nil {
			log.Fatalf("while parsing --info-metrics-labels: %v", err)
		}
		cfg.GaugeMetrics, err = cmd.Flags().GetInt("gauge-metrics")
		if err != nil {
			log.Fatalf("while parsing --gauge-metrics: %v", err)
		}

		reg := generator.CreateRegistry(cfg)
		server.ServeForever(reg)
	},
}

func Execute() {
	err := rootCmd.Execute()
	if err != nil {
		os.Exit(1)
	}
}

func init() {
	rootCmd.Flags().String("targets", "1", "Number of unique targets to generate. Can be a comma separated list of numbers in which case multiple labels will be used and each number specifies the number of unique values in that label.")
	rootCmd.Flags().Int("info-metrics-labels", 0, "Number of labels per info metric. If 0, no info-metric will be generated.")
	rootCmd.Flags().Int("gauge-metrics", 1, "Number of gauge metrics to generate for each target.")
	rootCmd.Flags().IntP("port", "p", 8080, "Port to listen on for scrape requests")
}

/*
 * Parse comma-separated list of target label counts into a slice of integers
 */
func parseTargets(targetsParam string) ([]int, error) {
	targets := make([]int, 0)
	for i, t := range strings.Split(targetsParam, ",") {
		tn, err := strconv.ParseInt(t, 10, 32)
		if err != nil {
			return nil, fmt.Errorf("\"%v\" element \"%s\" (#%d): %v", targetsParam, t, i, err)
		}
		if tn < 1 {
			return nil, fmt.Errorf("\"%v\" element \"%s\" (#%d): must be >= 1", targetsParam, t, i)
		}
		targets = append(targets, int(tn))
	}
	return targets, nil
}
