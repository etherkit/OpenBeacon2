#!/usr/bin/env python3

import serial
import serial.tools.list_ports
import sys
import os
import time
import threading
from timeit import default_timer as timer
import argparse
# import logging
# import atexit
import json
import cmd2
import colorama
from colorama import Fore, Back, Style

colorama.init()

JSON_MAX_SIZE = 500
PACKET_ID = b'\a'  # ASCII BEL
PACKET_TERM = b'\n'  # ASCII LF

modes = []
bands = []
band_modules = []
inst_band_modules = []

# logging.basicConfig(level=logging.DEBUG,
#                     format='[%(levelname)s] (%(threadName)-10s) %(message)s',
#                     )

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
                        timeout=1, writeTimeout=5)
except:
    print('Cannot open serial port ' + args.port)
    sys.exit(0)


# def keypress():
#     global char
#
#     while True:
#         char = getch()


def serial_handler():
    global modes, bands, band_modules, inst_band_modules, available_bands

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

            # Act on message
            if(message_type == 0x00): # Time Sync Request
                cur_time = time.time()
                while(cur_time == time.time()):
                    pass
                send_payload = {'timestamp': int(time.time())}
                # print(json.dumps(send_payload, ensure_ascii=True, separators=(',', ':')))
                send_serial_packet(1, json.dumps(
                    send_payload, ensure_ascii=True, separators=(',', ':')))
                CmdParser().async_alert("Time sync at " + time.asctime(time.gmtime()))
            elif(message_type == 0x03): # Parameter Response
                if(json_payload["config"] == 'mode'):
                    CmdParser().async_alert('{}: {}'.format(json_payload["config"], modes[json_payload["value"]]))
                elif(json_payload["config"] == 'band'):
                    CmdParser().async_alert('{}: {}'.format(json_payload["config"], bands[json_payload["value"]]["name"]))
                else:
                    CmdParser().async_alert('{}: {}'.format(json_payload["config"], json_payload["value"]))
            elif(message_type == 0x07): # Enumeration Response
                if 'modes' in json_payload:
                    modes = json_payload['modes']
                    # CmdParser().async_alert('{}'.format(modes))
                elif 'bands' in json_payload:
                    bands = json_payload['bands']
                    # CmdParser().async_alert('{}'.format(bands))
                    # CmdParser().async_alert('{}'.format(bands[7]['name']))
                elif 'band_modules' in json_payload:
                    band_modules = json_payload['band_modules']
                    # CmdParser().async_alert('{}'.format(band_modules))
                elif 'inst_band_modules' in json_payload:
                    band_modules = json_payload['inst_band_modules']
                    # CmdParser().async_alert('{}'.format(band_modules))
            elif(message_type == 0xFE): # Notification
                if(json_payload["text"] == 'TX Start'):
                    start = timer()
                    start_time = time.strftime("%H:%M:%S", time.gmtime())
                elif(json_payload["text"] == 'TX End'):
                    end = timer()
                    CmdParser().async_alert('Transmission at {} - {:.3f} s on {} Hz'.format(start_time, end - start, json_payload['data']))
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
    prompt = Style.BRIGHT + Fore.MAGENTA + '>' + Style.RESET_ALL + ' '
    intro = Style.BRIGHT + Fore.BLUE + 'OpenBeacon Mini' + Style.RESET_ALL

    def __init__(self):
        shortcuts = dict(self.DEFAULT_SHORTCUTS)
        shortcuts.update({'q': 'quit'})
        super().__init__(shortcuts=shortcuts)
        # cmd2.Cmd.__init__(self)

    set_parser = argparse.ArgumentParser()
    set_parser.add_argument('config', help='Configuration parameter to set')
    set_parser.add_argument('value', help='Configuration value')

    @cmd2.with_argparser(set_parser)
    def do_set(self, args):
        """Set an OpenBeacon Mini configuration parameter"""
        if args.config == 'mode':
            if args.value.upper() in modes:
                send_payload = {'config': args.config, 'set': True, 'value': modes.index(args.value.upper())}
                send_serial_packet(2, json.dumps(
                    send_payload, ensure_ascii=True, separators=(',', ':')))
        elif args.config == 'band':
            # TODO: get the most recent band modules first
            for index, m in enumerate(bands):
                if args.value.replace(' ', '').upper() == m['name'].replace(' ', '').upper(): # if the name of the band is found in the band table
                    if m['mod'] in band_modules: # if that band module is installed
                        send_payload = {'config': args.config, 'set': True, 'value': index}
                        send_serial_packet(2, json.dumps(
                            send_payload, ensure_ascii=True, separators=(',', ':')))
        else:
            send_payload = {'config': args.config, 'set': True, 'value': args.value}
            send_serial_packet(2, json.dumps(
                send_payload, ensure_ascii=True, separators=(',', ':')))

    get_parser = argparse.ArgumentParser()
    get_parser.add_argument('config', help='Configuration parameter to get')

    @cmd2.with_argparser(get_parser)
    def do_get(self, args):
        """Get an OpenBeacon Mini configuration parameter"""
        send_payload = {'config': args.config, 'get': True}
        send_serial_packet(2, json.dumps(
            send_payload, ensure_ascii=True, separators=(',', ':')))

    # enum_parser = argparse.ArgumentParser()
    # enum_parser.add_argument('enum', help='Configuration parameter to get')
    #
    # @cmd2.with_argparser(enum_parser)
    # def do_enum(self, args):
    #     send_payload = {'enum': args.enum}
    #     send_serial_packet(6, json.dumps(
    #         send_payload, ensure_ascii=True, separators=(',', ':')))

    list_parser = argparse.ArgumentParser()
    list_parser.add_argument('enum', help='Enumeration to list')

    @cmd2.with_argparser(list_parser)
    def do_list(self, args):
        """List valid values in an enumeration"""
        if args.enum == 'modes':
            CmdParser().async_alert(Style.BRIGHT + Fore.RED + 'Modes:' + Style.RESET_ALL)
            for m in modes:
                CmdParser().async_alert(m)
        elif args.enum == 'bands':
            for b in bands:
                if b['mod'] in band_modules:
                    CmdParser().async_alert(b['name'])

    tx_parser = argparse.ArgumentParser()

    # group = tx_parser.add_mutually_exclusive_group()
    tx_parser.add_argument('action', help='Enable or disable transmitting', choices=['enable', 'disable', 'cancel'])
    # group.add_argument('disable', help='Disable transmitting', nargs='?')

    @cmd2.with_argparser(tx_parser)
    def do_tx(self, args):
        """Enable or disable transmitting"""
        if args.action == 'enable':
            send_payload = {'action': 'tx_enable'}
            send_serial_packet(4, json.dumps(
                send_payload, ensure_ascii=True, separators=(',', ':')))
        elif args.action == 'disable':
            send_payload = {'action': 'tx_disable'}
            send_serial_packet(4, json.dumps(
                send_payload, ensure_ascii=True, separators=(',', ':')))
        elif args.action == 'cancel':
            send_payload = {'action': 'tx_cancel'}
            send_serial_packet(4, json.dumps(
                send_payload, ensure_ascii=True, separators=(',', ':')))


def main():
    # Start threads
    s = threading.Thread(target=serial_handler, name="Serial")
    s.daemon = True
    s.start()

    # Get the enums from OBM
    send_payload = {'enum': 'modes'}
    send_serial_packet(6, json.dumps(
        send_payload, ensure_ascii=True, separators=(',', ':')))
    send_payload = {'enum': 'bands'}
    send_serial_packet(6, json.dumps(
        send_payload, ensure_ascii=True, separators=(',', ':')))
    send_payload = {'enum': 'band_modules'}
    send_serial_packet(6, json.dumps(
        send_payload, ensure_ascii=True, separators=(',', ':')))
    send_payload = {'enum': 'inst_band_modules'}
    send_serial_packet(6, json.dumps(
        send_payload, ensure_ascii=True, separators=(',', ':')))

    # Drop into command parser loop
    CmdParser().cmdloop()


if __name__ == "__main__":
    main()
