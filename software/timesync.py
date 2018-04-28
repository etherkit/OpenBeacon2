import serial
import sys
import time

# Arduino serial dev paramaters
DEVICE = '/dev/ttyACM0' # Change this as necessary
BAUD = 57600

# Open serial port
try:
	ser = serial.Serial(port=DEVICE, baudrate=BAUD, timeout=1, writeTimeout=1)
except:
	print('Cannot open serial port')
	sys.exit(0)

#Wait for ASCII bell, then send the Unix time string
while True:
	ser_in = ser.read()
	if('\a' in ser_in):
		ser.write('T' + str(int(time.time())))
		print('Time sync at ' + str(int(time.time())))
