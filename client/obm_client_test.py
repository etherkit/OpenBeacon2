#!/usr/bin/python3

import sys
import serial
import json

BAUD = 115200
JSON_MAX_SIZE = 500
PACKET_ID = b'\a' # ASCII BEL
PACKET_TERM = b'\n' # ASCII LF

# Open serial port
try:
    ser = serial.Serial(port='/dev/ttyACM0', baudrate=BAUD,
                        timeout=1, writeTimeout=1)
    print('Serial port open')
except:
    print('Cannot open serial port ' + '/dev/ttyACM0')
    sys.exit(0)

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
    print(serial_packet)
    ser.write(serial_packet)

    return len(serial_packet)


def main():
    while True:
        if(ser.in_waiting > 0):
            ser_in = ser.read()
            if('\a' in ser_in.decode()): # Packet ID found
                # Get message type
                message_type = int.from_bytes(ser.read(), byteorder='big')
                print('Message type: {}'.format(message_type))

                # Determine payload length
                payload_len = int.from_bytes(ser.read(2), byteorder='big')
                print('Payload length: {}'.format(payload_len))

                # Get the payload
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
                # json_payload = json.loads(payload)
                # print(json_payload["name"])

                # Act on message
                if(message_type == 0):
                    send_serial_packet(1, "{\"name\":\"Noah\"}")


if __name__ == "__main__":
    main()
