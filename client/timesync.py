#!/usr/bin/env python3

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
import json
import cmd2
import colorama
from colorama import Fore, Back, Style

colorama.init()

JSON_MAX_SIZE = 500
PACKET_ID = b'\a'  # ASCII BEL
PACKET_TERM = b'\n'  # ASCII LF

logging.basicConfig(level=logging.DEBUG,
                    format='[%(levelname)s] (%(threadName)-10s) %(message)s',
                    )

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
    description="OpenBeacon Mini Control",
    epilog="Type 'quit' to exit")
arg_parser.add_argument(
    "-p", "--port", help="Serial port connected to OpenBeacon Mini", nargs='?', default=DEVICE)
arg_parser.add_argument(
    "-b", "--baud", help="Baud rate of the serial connection", nargs='?', default=BAUD)
arg_parser.add_argument(
    "-l", "--list-ports", help="Enumerate available serial ports", nargs=0, action=ListSerialPorts)
# arg_parser.add_argument("-v", "--verbose", action='store_true', help="Increase output verbosity")
arg_parser.add_argument("-v", "--verbose", default=0,
                        action="count", help="Increase output verbosity")
args = arg_parser.parse_args()
# Don't want to pass argv to cmd2, so let's delete it after parsing
sys.argv = [sys.argv[0]]


# Open serial port
try:
    ser = serial.Serial(port=args.port, baudrate=args.baud,
                        timeout=1, writeTimeout=1)
except:
    print('Cannot open serial port ' + args.port)
    sys.exit(0)


# def keypress():
#     global char
#
#     while True:
#         char = getch()


def serial_handler():
    global char

    while True:
        ser_in = ser.read()
        if('\a' in ser_in.decode()):
            # Get message type
            message_type = int.from_bytes(ser.read(), byteorder='big')
            # print('Message type: {}'.format(message_type))

            # Determine payload length
            payload_len = int.from_bytes(ser.read(2), byteorder='big')
            # print('Payload length: {}'.format(payload_len))

            # Get the payload
            payload = ""
            if(payload_len > 0):
                try:
                    payload = ser.read(payload_len).decode()
                except:
                    print('Payload malformed')
                    # sys.exit(0)

            # Make sure packet is terminated correctly
            try:
                ser_in = ser.read()
                '\n' in ser_in.decode()
            except:
                print('No packet terminator')
                # sys.exit(0)

            # Parse the payload
            # print(payload)
            if(payload_len > 0):
                json_payload = json.loads(payload)
            # print(json_payload["name"])

            # Act on message
            if(message_type == 0x00):
                send_payload = {'timestamp': int(time.time())}
                # print(json.dumps(send_payload, ensure_ascii=True, separators=(',', ':')))
                send_serial_packet(1, json.dumps(
                    send_payload, ensure_ascii=True, separators=(',', ':')))
                CmdParser().async_alert("Time sync at " + time.asctime(time.gmtime()))
            elif(message_type == 0xFE):
                if(json_payload["text"] == 'TX Start'):
                    start = timer()
                    start_time = time.strftime("%H:%M:%S - ", time.gmtime())
                elif(json_payload["text"] == 'TX End'):
                    end = timer()
                    CmdParser().async_alert(start_time + str(end - start))
                    # if args.verbosity >= 0: # TODO
                    # logging.info(start_time + str(end - start))

                if 'level' in json_payload:
                    if isinstance(json_payload["level"], int):
                        if args.verbose > json_payload["level"]:
                            CmdParser().async_alert(json_payload["text"])

        #     cur_time = time.time()
        #     while(cur_time == time.time()):
        #         pass
        #     time_str = "T" + str(int(time.time()))
        #     ser.write(time_str.encode())
        #     print("Time sync at " + time.asctime(time.gmtime()))
        # elif('\f' in ser_in.decode()):
        #     start = timer()
        #     start_time = time.strftime("%H:%M:%S - ", time.gmtime())
        #     while('\b' not in ser.read().decode()):
        #         pass
        #     end = timer()
        #     if args.verbosity > 0:
        #         logging.info(start_time + str(end - start))
        #         # print(start_time + str(end - start) + "\r")
        # elif('\v' in ser_in.decode()):
        #     if args.verbosity > 0:
        #         logging.info(ser.readline().decode())
        #         # print(ser.readline() + "\r")


def send_serial_packet(msg_type, payload):
    if(len(payload) > JSON_MAX_SIZE):
        return 0

    # Build packet header
    serial_packet = PACKET_ID
    serial_packet += msg_type.to_bytes(1, byteorder='big')
    serial_packet += len(payload).to_bytes(2, byteorder='big')

    # Add in payload
    serial_packet += payload.encode('ascii')

    # Append terminator char
    serial_packet += PACKET_TERM

    # Send it
    # print(serial_packet)
    ser.write(serial_packet)

    return len(serial_packet)


class CmdParser(cmd2.Cmd):
    prompt = '>'
    intro = Style.BRIGHT + Fore.BLUE + 'OpenBeacon Mini' + Style.RESET_ALL

    def __init__(self):
        # shortcuts = dict({})
        # shortcuts.update({'q': 'quit'})
        # super().__init__(self, shortcuts=shortcuts)
        cmd2.Cmd.__init__(self)

    # def do_v(self, args, unknown):
    #     pass

    # def do_quit():
    #     if args.verbosity > 0:
    #         logging.info('EXIT')
    #     sys.exit()


def main():
    # Start threads
    s = threading.Thread(target=serial_handler, name="Serial")
    s.daemon = True
    s.start()

    # Drop into command parser loop
    CmdParser().cmdloop()


if __name__ == "__main__":
    main()
