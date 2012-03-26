ERROR = -1
EMPTY = chr(0x40) 	# Status byte from BT if messagequeue is empty
IN = 11 		# In-queue in the BT
OUT = 1 		# Out-queue in the BT
BT_ADRESS = "00:06:66:03:A9:9C" # Hardware-address to the BT
PROGRAM = "fkub" # Default program to be run on the BT
CENTER_AREA = 1
LEFT_AREA = 2
RIGHT_AREA = 3

#Commands
MANUAL = 0x12
SEND_NEXT = 0x13
END = 0x14

CALIBRATE = chr(1)          # Constants for different functions in the BT-program
CALIBRATELEFT = chr(2)      # The first byte in the message to the BT is set to one of these
CALIBRATECENTER = chr(3)
CALIBRATERIGHT = chr(4)
NEXTLF = chr(5)
SETSPEED = chr(6)
SETMARGIN = chr(7)
LOCK = chr(8)
UNLOCK = chr(9)
STOP = chr(0x10)
RIGHT = chr(0x0f)
RIGHTFORW = chr(12)
LEFT = chr(0x0e)
LEFTFORW = chr(14)
STRAIGHT = chr(0x0c)
BACKWARD = chr(0x0d)
INCSPEED = chr(17)
DECSPEED = chr(18)
SETALGO = chr(19)

debug_nobluetooth = 1
debug = 2
