import serial
import sys
import time
from timeit import default_timer as timer

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
		#ser.reset_input_buffer()
		cur_time = int(time.time())
		while(cur_time == int(time.time())):
			pass
		ser.write('T' + str(int(time.time())))
		print('Time sync at ' + str(int(time.time())))
	elif('\f' in ser_in):
		start = timer()
		start_time = time.strftime("%H:%M:%S - ", time.gmtime())
		#ser.reset_input_buffer()
		while('\b' not in ser.read()):
			pass
		end = timer()
		print(start_time + str(end - start))
		#ser.reset_input_buffer()
		
