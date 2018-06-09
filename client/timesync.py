#!/usr/bin/python3

import serial
import serial.tools.list_ports
import sys, os
import time
import threading
import tty
import termios
from timeit import default_timer as timer
import argparse
import logging
import atexit

logging.basicConfig(level=logging.DEBUG,
                    format='[%(levelname)s] (%(threadName)-10s) %(message)s',
                    )



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

# Set up non-blocking keyboard read
try:
    from msvcrt import getch  # try to import Windows version
except ImportError:
    # import tty
    # import termios

    # fd = sys.stdin.fileno()
    # term = open(fd)

    def getch():   # define non-Windows version
        # old_settings = termios.tcgetattr(fd)
        # old_settings[3] = old_settings[3] | termios.ECHO
        # term = open(fd)
        # try:
            # tty.setraw(sys.stdin.fileno())
            # tty.setraw(fd, termios.TCSANOW)
            # tty.setcbreak(fd)
        ch = sys.stdin.read(1)
            # ch = term.read(1).decode()
            # ch = sys.stdin.read(1)
            # ch = os.read(fd, 1)
        # finally:
            # termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
            # termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
        # tty.setcbreak(fd, termios.TCSANOW)

        return ch

# Open serial port
try:
    ser = serial.Serial(port=args.port, baudrate=args.baud,
                        timeout=1, writeTimeout=1)
except:
    print('Cannot open serial port ' + args.port)
    sys.exit(0)


def keypress(ev):
    global char

    while not ev.is_set():
        char = getch()
        # if char is not None:
        #     if char == 'q' or char == b'q' or char == '\x1b' or char == b'\x1b':  # 1b is ESC
        #         print('EXIT')
        #         sys.exit(0)
        #     else:
        #         print('char: ')
        #     char = None

def serial_handler(ev):
    global char

    while not ev.is_set():
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
                print(start_time + str(end - start)  + "\r")
        elif('\v' in ser_in.decode()):
            if args.verbosity > 0:
                print(ser.readline() + "\r")


# # Start non-blocking keypress thread
# t = threading.Thread(target=keypress)
# t.daemon = True
# t.start()


def main():
    global char
    char = None
    # # Open serial port
    # try:
    #     ser = serial.Serial(port=args.port, baudrate=args.baud,
    #                         timeout=1, writeTimeout=1)
    # except:
    #     print('Cannot open serial port ' + args.port)
    #     sys.exit(0)

    # Start non-blocking keypress thread
    close_event = threading.Event()
    t = threading.Thread(target=keypress, args=(close_event,))
    s = threading.Thread(target=serial_handler, args=(close_event,))
    t.daemon = True
    s.daemon = True
    t.start()
    s.start()

    # Wait for ASCII bell, then send the Unix time string
    # while True:
    #     ser_in = ser.read()
    #     if('\a' in ser_in.decode()):
    #         cur_time = int(time.time())
    #         while(cur_time == int(time.time())):
    #             pass
    #         time_str = "T" + str(int(time.time()))
    #         ser.write(time_str.encode())
    #         print("Time sync at " + time.asctime(time.gmtime()) + "\r")
    #     elif('\f' in ser_in.decode()):
    #         start = timer()
    #         start_time = time.strftime("%H:%M:%S - ", time.gmtime())
    #         while('\b' not in ser.read().decode()):
    #             pass
    #         end = timer()
    #         if args.verbosity > 0:
    #             print(start_time + str(end - start)  + "\r")
    #     elif('\v' in ser_in.decode()):
    #         if args.verbosity > 0:
    #             print(ser.readline() + "\r")
    while True:
        if char is not None:
            if char == 'q' or char == b'q' or char == '\x1b' or char == b'\x1b':  # 1b is ESC
                print('EXIT' + '\r')
                close_event.set()
                os._exit(-1)

            char = None


if __name__ == "__main__":
    main()
