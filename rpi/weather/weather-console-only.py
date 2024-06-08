#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import time
import datetime
import glob
import logging
import sys
import pprint
import argparse
import csv

try:
    import bme680
except ImportError as ex:
    print("""
This script requires the bme680 module
Install with:
    sudo pip install -r requirements.txt
See README.md for details.
Error from Python import: {ex}
""".format(
        v="" if sys.version_info.major == 2 else sys.version_info.major,
        ex=ex
    ))
    sys.exit(1)

bme680_oversample_rates = {
        0: bme680.OS_NONE,
        1: bme680.OS_1X,
        2: bme680.OS_2X,
        4: bme680.OS_4X,
        8: bme680.OS_8X,
        16: bme680.OS_8X,
    }

def bme680_oversample(oversample_rate):
    rate = bme680_oversample_rates.get(oversample_rate, None)
    if rate is None:
        raise ValueError(f"invalid oversample rate {oversample_rate}, must be one of {list(bme680_oversample_rates.keys())}")
    return rate

def sensor_init(oversample_rate=4, temp_offset=0):
    # Set up BME680 sensor
    sensor = bme680.BME680()

    os_rate = bme680_oversample(oversample_rate)
    sensor.set_humidity_oversample(os_rate)
    sensor.set_pressure_oversample(os_rate)
    sensor.set_temperature_oversample(os_rate)
    sensor.set_filter(bme680.FILTER_SIZE_3)
    sensor.set_temp_offset(temp_offset)

    # Fetch sensor dating first so that device settings take effect
    sensor.get_sensor_data()

    return sensor

def print_sample(sensor_data, low_temp, high_temp, print_header=True):
    if print_header:
        sys.stdout.write("{:>10} {:>6} {:>10} {:>10} {:>10}\n".format("pressure", "rH", "temp", "low", "high"))
    sys.stdout.write(f"{sensor_data['pressure']:>10.2f} {sensor_data['humidity']:>6.2f} {sensor_data['temp']:>10.1f} {low_temp:>10.1f} {high_temp:>10.1f}\n")
    sys.stdout.flush()

def main():
    parser = argparse.ArgumentParser(description="datalogger")
    parser.add_argument("-i", "--sample-interval", type=float, default=10,
            help="Sample interval in fractional seconds")
    parser.add_argument("-p", "--print-nth-sample", type=int, default=1,
            help="Print every n'th sample to the console. Zero for never.")
    parser.add_argument("-f", "--output-dir", type=str,
            help="Write samples into CSV files in the nominated directory")
    parser.add_argument("--output-buffer-size", type=int, default=-1,
            help="Size in bytes of output buffer to use for csv output, -1 for auto, 0 for unbuffered, or 1 for line-buffered.")
    parser.add_argument("--output-flush-interval", type=int, default=600,
            help="Interval in seconds to flush the csv output buffer to disk. Only useful with large buffers.")
    parser.add_argument("-S", "--output-fsync-interval", type=int, default=3600,
            help="Force a fsync() to flush all changes to durable storage every N seconds")
    parser.add_argument("-H", "--headers-every", type=int, default=20,
            help="Repeat header every n'th line of output")
    parser.add_argument("-d", "--discard-first", type=int, default=0,
            help="Discard first N samples before starting to write to csv output. Rows are still output to console.")
    parser.add_argument("-A", "--altitude-meters", type=float, default=0,
            help="altitude in meters, to be written alongside samples")
    # OS_NONE (0) doesn't seem to work properly, so using min oversample of 1
    parser.add_argument("-O", "--oversample_rate", type=int, default=8, choices=[1,2,4,8,16],
            help="in-sensor oversample rate for pressure, temperature and humidity readings")
    parser.add_argument("-T", "--temperature-offset", type=float, default=0,
            help="fixed temperature offset in degrees C to apply")
    args = parser.parse_args()

    csv_writer = None
    csv_file = None
    if args.output_dir:
        output_path = os.path.join(args.output_dir, "{}.csv".format(int(time.time())))
        csv_file = open(output_path, 'w', newline='', buffering=args.output_buffer_size)
        csv_writer = csv.DictWriter(csv_file, fieldnames=['ts','runtime','temp','pressure','humidity'])
        csv_writer.writeheader()

        # TODO also write a json file with the settings used for oversample,
        # altitude, temp offset etc

    sensor = sensor_init(
            oversample_rate=args.oversample_rate,
            temp_offset=args.temperature_offset
            )

    start_time = time.time()

    # Initial values
    sensor_data = {
            'ts': start_time,
            'runtime': 0,
            'temp': sensor.data.temperature,
            'pressure': sensor.data.pressure,
            'humidity': sensor.data.humidity
            }
    curr_date = datetime.date.today().day
    low_temp = sensor.data.temperature
    high_temp = sensor.data.temperature

    last_fsync = start_time
    last_flush = start_time

    # TODO burn-in period?
    # TODO sample intervals

    # Main loop
    row_number = 0
    
    try:

        while True:
            ts = time.time()
            if sensor.get_sensor_data():
                logging.debug("sensor data: %s", pprint.pformat(sensor_data))
                temp = sensor.data.temperature
                if datetime.datetime.today().day == curr_date:
                    if temp < low_temp:
                        low_temp = temp
                    elif temp > high_temp:
                        high_temp = temp
                else:
                    curr_date = datetime.datetime.today().day
                    low_temp = temp
                    high_temp = temp
                sensor_data.update({
                    'ts': ts,
                    'runtime': round(time.time() - start_time, ndigits=3),
                    'temp': temp,
                    'pressure': sensor.data.pressure,
                    'humidity': sensor.data.humidity,
                })

                if args.print_nth_sample > 0 and row_number % args.print_nth_sample == 0:
                    print_header = (args.headers_every > 0) and (row_number // args.print_nth_sample) % args.headers_every == 0
                    print_sample(sensor_data, low_temp, high_temp, print_header=print_header)

                if csv_writer:
                    csv_writer.writerow(sensor_data)
            else:
                logging.info("sensor data acquire failed")


            if args.output_fsync_interval > 0 and ts > last_fsync + args.output_fsync_interval:
                # always flush before fsync
                csv_file.flush()
                os.fsync(csv_file)
                last_flush = ts
                last_fsync = ts
            elif args.output_flush_interval > 0 and ts > last_flush + args.output_flush_interval:
                csv_file.flush()
                last_flush = ts

            row_number += 1
            time.sleep(args.sample_interval)
    finally:
        if csv_file:
            csv_file.close()

if __name__ == '__main__':
    logging.basicConfig(level=os.environ.get("LOGLEVEL", "WARNING"))
    main()

# vim: sw=4 ts=4 ai et si
