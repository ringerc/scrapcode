#include <avr/power.h>
#include <avr/io.h>

const int SAMPLES_TO_AVERAGE = 10;
const int SENSOR_ANALOG_PIN = 0;
const int INTERSAMPLE_SLEEP_TIME_MS = 20;
const int DELAY_BETWEEN_SAMPLE_REPORTS_MS = 2000;

void setup() {
  power_spi_disable();
  Serial.begin(9600);
}
  
void loop() {
  static short samples[10];
  static int lastSample=0, nextSample = 0;
  float mean, stdDev;
  int msDelayed = 0;
  // Sample at a high rate until we get a reasonably stable reading
  // to report.
  do {
    // Capture the next sample using the samples array as a
    // ring buffer.
    if (nextSample == SAMPLES_TO_AVERAGE)
      nextSample = 0;
    samples[nextSample++] = analogRead(SENSOR_ANALOG_PIN);
    // Grab the stats from the samples - a mean to report
    // and a standard deviation to decide whether the rate
    // of change and/or size of error is too great to bother
    // reporting this result.
    calcSampleStats(samples, mean, stdDev);
    delay(INTERSAMPLE_SLEEP_TIME_MS);
    // Intersample sleep time can overflow, but if it does
    // things are busted enough that we have worse problems.
    msDelayed += INTERSAMPLE_SLEEP_TIME_MS;
  } while (!reportResult(mean, stdDev));
  // Having got a good reading, go to sleep until 
  // we're ready to take the next one.
  
  delay(36000); // TEN MINUTE delay
}

void calcSampleStats(const short * const samples, float & sampleMean, float & stdDev) {
  int sampleSum = 0;
  // Find the mean of the humidities
  for (int i = 0; i < SAMPLES_TO_AVERAGE; i++) {
    sampleSum = sampleSum + samples[i];
  }
  sampleMean = float(sampleSum)/10;
  // Then the sum of the squares of the differences
  // between each sample and the mean:
  float sqDevSum = 0;
  for (int i = 0; i < SAMPLES_TO_AVERAGE; i++) {
    sqDevSum += pow(sampleMean - float(samples[i]), 2);
  }
  // Find the square root of the above divided by the
  // number of samples:
  stdDev = sqrt(sqDevSum/SAMPLES_TO_AVERAGE);
}

boolean reportResult(float & mean, float & stdDev) {  
  static boolean lastResultGood = true;
  if (stdDev < 5) {
    Serial.print("\nMean:");
    Serial.print(mean);
    Serial.print(", Std. Dev: ");
    Serial.print(stdDev);
    lastResultGood = true;
  } else {
    if (lastResultGood) {
      Serial.print("\nWaiting for value to stabilize: ");
    } else {
      Serial.print(".");
    }
    lastResultGood = false;
  }
  return lastResultGood;
}