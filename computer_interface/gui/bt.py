from constants import *
from bluetooth import *
import time

ERROR = -1
IN = 11 		# In-queue in the BT
OUT = 1 		# Out-queue in the BT
BT_ADRESS = "00:06:66:03:A9:9C" # Hardware-address to the BT
PROGRAM = "fkub" # Default program to be run on the BT
DEFAULT = 0xff
CENTER_AREA = 1
LEFT_AREA = 2
RIGHT_AREA = 3

##
# Class which handles communication with the BT
# Has functions for receiving and sending data
# and starting and stopping programs
# Also creates the connection to the BT

class Cbt:
  ##
  # Constructor, addr is a string with the physical address of the BT
  # Creates socket
  def __init__(self, addr, send_socket_lock, recv_socket_lock) : 
    print debug_nobluetooth
    if debug_nobluetooth: 	# Debug for running without bluetooth
      self.bt = 1
      return

    self.bt = BluetoothSocket( RFCOMM ) 	# Create socket
    error = self.bt.connect((addr, 1)); 	# Connect, channel 1
    self.bt.setblocking(1) 		# Activates blocking, which makes the socket wait for data if there is none
    self.send_socket_lock = send_socket_lock
    self.recv_socket_lock = recv_socket_lock
    if (not(error)) : 
      print("Bluetooth connection OK")                 # Everything is fine we've got a connection to the BT 
    else : 
      print ("Didn't find brick, error code: ", error) # Outputs error, but bt.connect() throws exceptions which are unhandled so it's mostly useless
  ##
  # Returns 0 if all is good
  # Otherwise ERROR
  # Used by Crobot to know when to exit
  def error(self):
    if debug_nobluetooth: # Debug
      if self.bt == 1 : return 0
      else : return ERROR
    if self.bt.fileno()==ERROR: return ERROR
    else : return 0	

  ##
  # Receives data from the BT
  # Returns the received command
  #
  # Removes length bytes from packet
  def readcmd(self):
    if debug :
      print 'readcmd'
    if debug_nobluetooth:
      time.sleep(1)
      return str('' + chr(CMD_SENSOR_DATA)+IR_LONG_LEFT+chr(23))   # Debug, we don't have a bluetooth-connection so pretend that everything is fine

#    self.recv_socket_lock.acquire() 		# Lock socket
    if self.bt.fileno() == ERROR : # Connection down
#      self.recv_socket_lock.release()
      return ERROR 
    head = self.bt.recv(2)                 # Read length of packet and command
    if(len(head) < 2):
      head += self.bt.recv(1)
    data = ''
    while (ord(head[1]) > len(data)):
      data += self.bt.recv(ord(head[1]) - len(data))       # Recieve packet
#    self.recv_socket_lock.release() 	# Release lock
    return head[0] + data                   # Strip length byte.

  ##
  # Sends data to the BT
  # parameter cmd = command to be sent
  #
  # Adds length data in the beginning of packet
  def sendcmd(self, cmd, data):
    if debug : 
      s = 'sendcmd: ' + str(cmd)
      for b in data:
        s += ' ' + str(ord(b))
      print s
    if debug_nobluetooth: return
#    self.send_socket_lock.acquire() 		# Lock socket
    if self.bt.fileno() == ERROR : # Connection down
#      self.send_socket_lock.release()
      return ERROR 
    self.bt.send(chr(cmd))
    self.bt.send(chr(len(data))) 		# Length of data
    if len(data) != 0 : 
      self.bt.send(str(data))			# Data
#    self.send_socket_lock.release() 	# Release lock

  ##
  # Get data from BT
  # Returns 

  ##
  # Sends message to the BTs message queue
  # parameter message = string with message to be sent
  #
  # Response:
  # Byte 0: 0x02
  # Byte 1: 0x09
  # Byte 2: Status
  """def sendmessage(self, message):
    if debug :
      for char in message : 
        print " " + hex(ord(char)),
      print ""
    if debug_nobluetooth: return chr(0x02) # Debug
    queue = OUT 			# The queue we are sending to
    socket_lock.acquire() 		# Lock socket
    if self.bt.fileno() == ERROR : # Connection down
      socket_lock.release()
      return ERROR 
    cmd = chr( 0x80 ) + chr( 0x09 ) + chr( queue ) + chr( len(message)+1  ) + message + chr(0x00) # Command
    self.sendcmd(cmd) 	# Send
    #result = self.response() # Get response
    socket_lock.release() 	# Release lock
    return 	# Return OK"""

  ##
  # Reads a message from the BTs queue
  # Returns a string with the message
  # 
  # MESSAGEREAD
  # Byte 0: 0x00 Want response
  # Byte 1: 0x13 Command
  # Byte 2: Remote inbox number, add 10 to the queue number that is set by SendMessage() in NXC, outgoing messages has queuenumbers 10-19
  # Byte 3: Local inbox number (0-9), doesn't really matter since we are not a BT
  # Byte 4: Remove? (Boolean (TRUE) non-zero, clears message from remote inbox
  #
  # Response:
  # Byte 0: 0x02
  # Byte 1: 0x13
  # Byte 2: Status
  # Byte 3: Local inbox number (0-9) (should be byte 3 from the sent command)
  # Byte 4: Message size
  # Byte 5-63: Message data (padded)
  """def readmessage(self):
    if debug_nobluetooth: # Debug
      result = chr(0x01)+chr(0x02)+chr(0x03)+chr(0x04)+chr(0x05)+chr(0x06)+chr(0x07)+chr(0x08)+chr(0x09)+chr(0x0a) # Respond with some random data if we're testing
      return result
    queue = IN 		# Queue we are receiving from should be the queue we called in SendMessage() in the NXC-program + 10
    socket_lock.acquire() 	# Lock socket
    if self.bt.fileno() == ERROR : # Connection down
      socket_lock.release()
      return ERROR

    cmd = chr(0x00) + chr(0x13) + chr(queue) + chr(0x00) + chr(0x01) # Command
    self.sendcmd(cmd) 		# Send
    result = self.response()	# Get response
    socket_lock.release() 		# Release socket
    if (result[0] != chr(0x02) or result[1] != chr(0x13) or result[3] != chr(0x00)) : return ERROR # Check that everything is alright
    elif (result[2] != chr(0x00) and result[2] != chr(0x40)) : # Print error message to console if status isn't good
      print("Status read not OK: ", hex(ord(result[2])))
      return ord(result[2])
    length = ord(result[4]) 
    return result[5:length+4] # Return message data"""

  ##
  # Send calibration data to robot
  # 
  """def calibrate(self, left, center, right):
    if debug_nobluetooth: 
      print "calibrate" + chr(left) + chr(center) + chr(right)
      return chr(0x02)
    message = chr(CALIBRATE) + chr(left) + chr(center) + chr(right)
    return self.sendmessage(message)"""

  ##
  # Closes the bluetooth-connection
  # 
  def shutdown(self):
    if debug_nobluetooth: # Debug set bt to 0 so the threads exits nicely
      self.bt = 0
      return
#    self.socket_lock.acquire() # Lock socket
    if self.bt.fileno() == ERROR:
      self.socket_lock.release() 
      return
    self.bt.shutdown(2) 	# Stop accepting data
    self.bt.close() 	# Destroy socket
#    self.socket_lock.release() 	# Release socket
