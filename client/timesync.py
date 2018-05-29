#!/usr/bin/python3

import serial
import sys
import time
from timeit import default_timer as timer
import argparse
#import fire

# Arduino serial dev paramaters
DEVICE = '/dev/ttyACM0'  # Change this as necessary
BAUD = 57600

# Set up argument parser
arg_parser = argparse.ArgumentParser(description = "OpenBeacon Mini Synchronization")
arg_parser.add_argument("--port", help = "Serial port that OpenBeacon Mini is connected to", default = DEVICE)
arg_parser.add_argument("--baud", help = "Baud rate of the serial connection", default = BAUD)
args = arg_parser.parse_args()

def main():
    # Open serial port
    try:
        ser = serial.Serial(port=DEVICE, baudrate=BAUD,
                            timeout=1, writeTimeout=1)
    except:
        print('Cannot open serial port')
        sys.exit(0)

    # Handle the args


    # Wait for ASCII bell, then send the Unix time string
    while True:
        ser_in = ser.read()
        if('\a' in ser_in.decode()):
            # ser.reset_input_buffer()
            cur_time = int(time.time())
            while(cur_time == int(time.time())):
                pass

            time_str = "T" + str(int(time.time()))
            ser.write(time_str.encode())
            print('Time sync at ' + str(int(time.time())))
        elif('\f' in ser_in.decode()):
            start = timer()
            start_time = time.strftime("%H:%M:%S - ", time.gmtime())
            # ser.reset_input_buffer()
            while('\b' not in ser.read()):
                pass
            end = timer()
            print(start_time + str(end - start))
            # ser.reset_input_buffer()
        elif('\v' in ser_in.decode()):
            print(ser.readline())


if __name__ == "__main__":
    main()
