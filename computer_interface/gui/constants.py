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
CMD_REG_PARAMS = 0x18
CMD_AUTO_ON = 0x19
CMD_SET_REG_PARAMS = 0x20
CMD_SET_MODE = 0x21

# Messages
SEND_NEXT = chr(CMD_SEND_NEXT) + chr(0)
EMPTY = chr(CMD_END) + chr(0) 	# Message from plogen if end of queue

#Sensor constants
IR_SHORT_LEFT = chr(1)
IR_SHORT_RIGHT = chr(2)
IR_ANGLE = chr(3)
IR_DIFF = chr(4)
AUTO_MODE = chr(5)
TAPE = chr(6)
TAPE_VALUE = chr(7)
IR_LONG_LEFT = chr(8)
IR_LONG_RIGHT = chr(9)
IR_SHORT_BACK = chr(10)

#Control constants
LEFTFORW = chr(0x0a)
RIGHTFORW = chr(0x0b)
FORWARD = chr(0x0c)
BACKWARD = chr(0x0d)
LEFT = chr(0x0e)
RIGHT = chr(0x0f)
STOP = chr(0x10)
OPEN = chr(0x11)
CLOSE = chr(0x12)

#Regulator constants
REG_P = chr(1)
REG_D = chr(2)
REG_SPEED = chr(3)
REG_TIMER = chr(4)

#Constants for setMode

debug_nobluetooth = 1
debug = 0
