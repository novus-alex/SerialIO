import serialio

class Serial(object):
	def __init__(self, port, baudrate, timeout):
		self.port = port
		self.baudrate = baudrate
		self.timeout = timeout
		self._openPort()

	def _openPort(self):
		self.hComm = serialio.Serial(self.port, self.baudrate)		# Opening the port

	def read(self):
		data = serialio.read(self.hComm)				# Listening to serial port
		splited = data.split() 						# To remove \r\n(\n)
		return splited[0] 						# Returning the data

	def write(self, msg):
		serialio.write(self.hComm, msg)

	def close(self):
		serialio.close(self.hComm)

ser = Serial("COM3", 9600, 1)
ser.write("test")
data = ser.read()
print(data)
ser.close()
