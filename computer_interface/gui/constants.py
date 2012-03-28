ERROR = -1
IN = 11 		# In-queue in the BT
OUT = 1 		# Out-queue in the BT
BT_ADRESS = "00:06:66:03:A9:9C" # Hardware-address to the BT
PROGRAM = "fkub" # Default program to be run on the BT
CENTER_AREA = 1
LEFT_AREA = 2
RIGHT_AREA = 3

# Commands
CMD_MANUAL = 0x12
CMD_SEND_NEXT = 0x13
CMD_END = 0x14
CMD_SENSOR_DATA = 0x15

# Messages
SEND_NEXT = chr(CMD_SEND_NEXT) + chr(0)
EMPTY = chr(CMD_END) + chr(0) 	# Message from plogen if end of queue

#Sensor constants
IRLEFT = chr(1)
IRRIGHT = chr(2)
IRANGLE = chr(3)
IRDIFF = chr(4)
AUTO_MODE = chr(5)
TAPE = chr(6)

#Control constants
STOP = chr(0x10)
RIGHT = chr(0x0f)
RIGHTFORW = chr(12)
LEFT = chr(0x0e)
LEFTFORW = chr(14)
FORWARD = chr(0x0c)
BACKWARD = chr(0x0d)

debug_nobluetooth = 0
debug = 2
