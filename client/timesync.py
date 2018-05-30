#!/usr/bin/python3

import serial
import serial.tools.list_ports
import sys
import time
import threading
from timeit import default_timer as timer
import argparse

# Arduino serial dev paramaters
if sys.platform.startswith('win'):
    DEVICE = 'COM1'
elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
    DEVICE = '/dev/ttyACM0'
elif sys.platform.startswith('darwin'):
    DEVICE = '/dev/tty.'
else:
    raise EnvironmentError('Unsupported platform')
    # DEVICE = '/dev/ttyACM0'

BAUD = 57600


class ListSerialPorts(argparse.Action):
    def __init__(self, option_strings, dest, nargs=0, **kwargs):
        super(ListSerialPorts, self).__init__(
            option_strings, dest, nargs=nargs, **kwargs)

    def __call__(self, parser, namespace, values, option_string=None):
        print("Available serial ports:")
        for port in serial.tools.list_ports.comports():
            print(str(port.device))
        sys.exit(0)


# Set up argument parser
arg_parser = argparse.ArgumentParser(
    description="OpenBeacon Mini Synchronization",
    epilog="Press Escape or Q to quit")
arg_parser.add_argument(
    "--port", "-p", help="Serial port connected to OpenBeacon Mini", nargs='?', default=DEVICE)
arg_parser.add_argument(
    "--baud", "-b", help="Baud rate of the serial connection", nargs='?', default=BAUD)
arg_parser.add_argument(
    "--list-ports", "-l", help="Enumerate available serial ports", nargs=0, action=ListSerialPorts)
arg_parser.add_argument("--verbosity", "-v",
                        action="store_true", help="Increase output verbosity")
args = arg_parser.parse_args()

# Set up non-blocking keyboard read
try:
    from msvcrt import getch  # try to import Windows version
except ImportError:
    import tty
    import termios

    def getch():   # define non-Windows version
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        try:
            tty.setraw(sys.stdin.fileno())
            ch = sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
        return ch


def keypress():
    global char
    char = getch()


# Start non-blocking keypress thread
t = threading.Thread(target=keypress)
t.daemon = True
t.start()


def main():
    global char
    char = None

    # Open serial port
    try:
        ser = serial.Serial(port=args.port, baudrate=args.baud,
                            timeout=1, writeTimeout=1)
    except:
        print('Cannot open serial port ' + args.port)
        sys.exit(0)

    # Wait for ASCII bell, then send the Unix time string
    while True:
        ser_in = ser.read()
        if('\a' in ser_in.decode()):
            cur_time = int(time.time())
            while(cur_time == int(time.time())):
                pass
            time_str = "T" + str(int(time.time()))
            ser.write(time_str.encode())
            print("Time sync at " + time.asctime(time.gmtime()) + "\r")
        elif('\f' in ser_in.decode()):
            start = timer()
            start_time = time.strftime("%H:%M:%S - ", time.gmtime())
            while('\b' not in ser.read().decode()):
                pass
            end = timer()
            if args.verbosity == 2:
                print(start_time + str(end - start))
        elif('\v' in ser_in.decode()):
            if args.verbosity == 2:
                print(ser.readline())

        if char is not None:
            if char == 'q' or char == b'q' or char == '\x1b' or char == b'\x1b':  # 1b is ESC
                sys.exit(0)
            char = None
            t = threading.Thread(target=keypress)
            t.daemon = True
            t.start()


if __name__ == "__main__":
    main()
