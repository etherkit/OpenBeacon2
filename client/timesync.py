#!/usr/bin/python3

import serial
import serial.tools.list_ports
import sys
import os
import time
import threading
from timeit import default_timer as timer
import argparse
import logging
import atexit

logging.basicConfig(level=logging.DEBUG,
                    format='[%(levelname)s] (%(threadName)-10s) %(message)s',
                    )

# Set up non-blocking keyboard read
try:
    from msvcrt import getch  # try to import Windows version
except ImportError:
    import tty
    import termios
    import fcntl

    def getch():   # define non-Windows version
        with raw(sys.stdin):
            with nonblocking(sys.stdin):
                ch = sys.stdin.read(1)

        return ch

if sys.platform.startswith('linux') or sys.platform.startswith('cygwin') or sys.platform.startswith('darwin'):
    class raw(object):
        def __init__(self, stream):
            self.stream = stream
            self.fd = self.stream.fileno()

        def __enter__(self):
            self.original_stty = termios.tcgetattr(self.stream)
            tty.setcbreak(self.stream)

        def __exit__(self, type, value, traceback):
            termios.tcsetattr(self.stream, termios.TCSANOW, self.original_stty)

    class nonblocking(object):
        def __init__(self, stream):
            self.stream = stream
            self.fd = self.stream.fileno()

        def __enter__(self):
            self.orig_fl = fcntl.fcntl(self.fd, fcntl.F_GETFL)
            fcntl.fcntl(self.fd, fcntl.F_SETFL, self.orig_fl | os.O_NONBLOCK)

        def __exit__(self, *args):
            fcntl.fcntl(self.fd, fcntl.F_SETFL, self.orig_fl)

# Arduino serial dev paramaters
if sys.platform.startswith('win'):
    DEVICE = 'COM1'
    fd = None
elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
    DEVICE = '/dev/ttyACM0'
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(sys.stdin)
    # old_settings[3] = old_settings[3] | termios.ECHO
    # old_settings[3] = old_settings[3] | termios.ICANON
    tty.setcbreak(fd)
elif sys.platform.startswith('darwin'):
    DEVICE = '/dev/tty.'
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)
    tty.setcbreak(fd)
else:
    raise EnvironmentError('Unsupported platform')
    # DEVICE = '/dev/ttyACM0'


def set_normal_term():
    if sys.platform.startswith('win'):
        pass
    else:
        termios.tcsetattr(sys.stdin, termios.TCSADRAIN, old_settings)


atexit.register(set_normal_term)

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
arg_parser.add_argument("--verbosity", "-v", default=0,
                        action="count", help="Increase output verbosity")
args = arg_parser.parse_args()


# Open serial port
try:
    ser = serial.Serial(port=args.port, baudrate=args.baud,
                        timeout=1, writeTimeout=1)
except:
    print('Cannot open serial port ' + args.port)
    sys.exit(0)


def keypress():
    global char

    while True:
        char = getch()


def serial_handler():
    global char

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
            if args.verbosity > 0:
                print(start_time + str(end - start) + "\r")
        elif('\v' in ser_in.decode()):
            if args.verbosity > 0:
                print(ser.readline() + "\r")


def main():
    global char
    char = None

    # Start threads
    # t = threading.Thread(target=keypress)
    s = threading.Thread(target=serial_handler)
    # t.daemon = True
    s.daemon = True
    # t.start()
    s.start()

    while True:
        char = getch()
        if char == 'q' or char == b'q' or char == '\x1b' or char == b'\x1b':  # 1b is ESC
            print('EXIT' + '\r')
            sys.exit()

        char = None


if __name__ == "__main__":
    main()
