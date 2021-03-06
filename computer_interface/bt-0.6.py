#! /usr/bin/python
from bluetooth import *	# Bluetooth-connection
import time 		# Sleep()
from Tkinter import * 	# GUI
import threading 	# Threading
import pygame  		# For keyboard input

ERROR = -1
EMPTY = chr(0x40) 	# Status byte from NXT if messagequeue is empty
IN = 11 		# In-queue in the NXT
OUT = 1 		# Out-queue in the NXT
NXT_ADRESS = "00:16:53:05:3c:cc" # Hardware-address to the NXT
PROGRAM = "fkub" # Default program to be run on the NXT
DEFAULT = 0xff
CENTER_AREA = 1
LEFT_AREA = 2
RIGHT_AREA = 3

CALIBRATE = chr(1)          # Constants for different functions in the NXT-program
CALIBRATELEFT = chr(2)      # The first byte in the message to the NXT is set to one of these
CALIBRATECENTER = chr(3)
CALIBRATERIGHT = chr(4)
NEXTLF = chr(5)
SETSPEED = chr(6)
SETMARGIN = chr(7)
LOCK = chr(8)
UNLOCK = chr(9)
STILL = chr(10)
RIGHT = chr(11)
RIGHTFORW = chr(12)
LEFT = chr(13)
LEFTFORW = chr(14)
STRAIGHT = chr(15)
BACKWARD = chr(16)
INCSPEED = chr(17)
DECSPEED = chr(18)
SETALGO = chr(19)

socket_lock = threading.Semaphore() 	# Semaphore to handle different threads using the bluetooth-socket
debug_nobluetooth = 1			        # Run without bluetooth, debug
debug = 2                               # Turns on or off some printing to console. 1 = Print outgoing data, 2 = Print in and outgoing data

##
# Class which handles communication with the NXT
# Has functions for receiving and sending data
# and starting and stopping programs
# Also creates the connection to the NXT

class Cnxt:
	##
	# Constructor, addr is a string with the physical address of the NXT
	# Creates socket
	def __init__(self, addr) : 
		print debug_nobluetooth
		if debug_nobluetooth: 	# Debug for running without bluetooth
			self.nxt = 1
			return
	
		self.nxt = BluetoothSocket( RFCOMM ) 	# Create socket
		error = self.nxt.connect((addr, 1)); 	# Connect, channel 1
		self.nxt.setblocking(1) 		# Activates blocking, which makes the socket wait for data if there is non
		if (not(error)) : 
			print("Bluetooth connection OK")                 # Everything is fine we've got a connection to the NXT 
		else : 
			print ("Didn't find brick, error code: ", error) # Outputs error, but nxt.connect() throws exceptions which are unhandled so it's mostly useless
	##
	# Returns 0 if all is good
	# Otherwise ERROR
	# Used by Crobot to know when to exit
	def error(self):
		if debug_nobluetooth: # Debug
			if self.nxt == 1 : return 0
			else : return ERROR
		if self.nxt.fileno()==ERROR: return ERROR
		else : return 0	
	
	##
	# Receives data from the NXT
	# Returns the received command
	#
	# Removes length bytes from packet
	def response(self):
		if debug_nobluetooth: return chr(0x00)    # Debug, we don't have a bluetooth-connection so pretend that everything is fine
		length = self.nxt.recv(2)                 # Read length of packet
		cmd = self.nxt.recv(ord(length[0]))       # Recieve packet
		return cmd 
	
	##
	# Sends data to the NXT
	# parameter cmd = command to be sent
	#
	# Adds length data in the beginning of packet
	def sendcmd(self, cmd):
		if debug_nobluetooth: return
		length = len(cmd)
		self.nxt.send(chr(len(cmd))) 		# LSB of packetlength
		self.nxt.send(chr(0x00))		# MSB of packetlength always zero, since no packets in NXT protocol is longer than 256 bytes
		self.nxt.send(cmd)			# Data

	##
	# Runs a program on the NXT
	# 
	# parameter program = string with filename of file to be run
	# Returns status-byte
	# 
	# STARTPROGRAM
	# Byte 0: 0x00 (or 0x08 if we don't want a response) same for all packets with directcommands
	# Byte 1: 0x00 Command
	# Byte 2-21: Filename, ASCIIZ-string max-size [15.3] + NULL-char
	#
	# Response:
	# Byte 0: 0x02
	# Byte 1: 0x01
	# Byte 2: Status 
	def startprogram(self, program) :
		if debug : print "Start: " + program
		if debug_nobluetooth: return chr(0x00) # Debug 
		socket_lock.acquire()
		if self.error() == ERROR: # Connection is down 
			socket_lock.release()
			return ERROR
		
		cmd = chr(0x00) + chr(0x00) + program + chr(0x00) # Command
		self.sendcmd(cmd) 	# Send
		time.sleep(0.01)
		result = self.response() # Get response
		socket_lock.release()
		return result[2] 	# Return status-byte
	
	##
	# Stops the running program on the NXT
	#
	# Returns status-byte
	#
	# STOPPROGRAM
	# Byte 0: 0x00 Want response
	# Byte 1: 0x01 Command
	#
	# Response:
	# Byte 0: 0x02
	# Byte 1: 0x01
	# Byte 2: Status
	def stopprogram(self) :
		if debug_nobluetooth: return chr(0x00) # Debug
		socket_lock.acquire() 		# Lock socket
		if self.error() == ERROR: 	# Connection down
			socket_lock.release()
			return ERROR
		
		cmd = chr(0x00) + chr(0x01) 	#Command
		self.sendcmd(cmd) 		# Send
		time.sleep(0.01)
		result = self.response() 	# Get response
		socket_lock.release() 		# Release socket
		if result[0] == chr(0x02) and result[1] == chr(0x01) : return result[2] # Check that everything is okay and return status-byte
		else : return ERROR
	
	##
	# Sends message to the NXTs message queue
	# parameter message = string with message to be sent
	#
	# MESSSAGEWRITE
	# Byte 0: 0x00 Want response
	# Byte 1: 0x09 Command
	# Byte 2: Inbox number (0-9)
	# Byte 3: Messagesize
	# Byte 4 - N: Message data, N = Messagesize + 3
	# 
	# Response:
	# Byte 0: 0x02
	# Byte 1: 0x09
	# Byte 2: Status
	def sendmessage(self, message):
		if debug :
			for char in message : 
				print " " + hex(ord(char)),
			print ""
		if debug_nobluetooth: return chr(0x02) # Debug
		queue = OUT 			# The queue we are sending to
		socket_lock.acquire() 		# Lock socket
		if self.nxt.fileno() == ERROR : # Connection down
			socket_lock.release()
			return ERROR 
		cmd = chr( 0x80 ) + chr( 0x09 ) + chr( queue ) + chr( len(message)+1  ) + message + chr(0x00) # Command
		self.sendcmd(cmd) 	# Send
		#result = self.response() # Get response
		socket_lock.release() 	# Release lock
		return 	# Return OK
	
	##
	# Reads a message from the NXTs queue
	# Returns a string with the message
	# 
	# MESSAGEREAD
	# Byte 0: 0x00 Want response
	# Byte 1: 0x13 Command
	# Byte 2: Remote inbox number, add 10 to the queue number that is set by SendMessage() in NXC, outgoing messages has queuenumbers 10-19
	# Byte 3: Local inbox number (0-9), doesn't really matter since we are not a NXT
	# Byte 4: Remove? (Boolean (TRUE) non-zero, clears message from remote inbox
	#
	# Response:
	# Byte 0: 0x02
	# Byte 1: 0x13
	# Byte 2: Status
	# Byte 3: Local inbox number (0-9) (should be byte 3 from the sent command)
	# Byte 4: Message size
	# Byte 5-63: Message data (padded)
	def readmessage(self):
		if debug_nobluetooth: # Debug
			result = chr(0x01)+chr(0x02)+chr(0x03)+chr(0x04)+chr(0x05)+chr(0x06)+chr(0x07)+chr(0x08)+chr(0x09)+chr(0x0a) # Respond with some random data if we're testing
			return result
		queue = IN 		# Queue we are receiving from should be the queue we called in SendMessage() in the NXC-program + 10
		socket_lock.acquire() 	# Lock socket
		if self.nxt.fileno() == ERROR : # Connection down
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
		return result[5:length+4] # Return message data

	##
	# Send calibration data to robot
	# 
	def calibrate(self, left, center, right):
		if debug_nobluetooth: 
			print "calibrate" + chr(left) + chr(center) + chr(right)
			return chr(0x02)
		message = chr(CALIBRATE) + chr(left) + chr(center) + chr(right)
		return self.sendmessage(message)
	
	##
	# Closes the bluetooth-connection
	# 
	def shutdown(self):
		if debug_nobluetooth: # Debug set nxt to 0 so the threads exits nicely
			self.nxt = 0
			return
		socket_lock.acquire() # Lock socket
		if self.nxt.fileno() == ERROR:
			socket_lock.release() 
			return
		self.nxt.shutdown(2) 	# Stop accepting data
		self.nxt.close() 	# Destroy socket
		socket_lock.release() 	# Release socket

##
# Wrapper for Label class which reads and display data from the robot object
# 
# parameters 
# frame = parent frame
# robot = robot object
# textvarible = StringVar that holds text to be displayed
# 
# This object handles threading problems.
# The readsensors-thread writes data to the robot-object
# updateData() in this object reads data from the robot object and displays it every 100 milliseconds
class ThreadSafeLabel(Label):

	##
	# Constructor
	# Initiates saves variables and initiates the inherited Label
	def __init__(self, frame, robot):
		self.data = StringVar() # Create StringVar to hold text
		self.data.set("No data") # Initiate
		Label.__init__(self, frame, textvariable = self.data) # Call parents constructor
		self.robot = robot 	# Save robot object 
		self.updateData() 	# Call updateData() to start the "updateloop"

	def updateData(self) :
		# Display data from the robot-object on the label
		self.data.set("Light: " + str(self.robot.getLight()) + \
			"\nUltrasound: " + str(self.robot.getUs()) + \
			"\nTouch: " + str(self.robot.getTouch()) + \
			"\nBT lock: " + str(self.robot.getLock()) + \
			"\nSpeed: " + str(self.robot.getSpeed()) + \
			"\nLeft: " + str(self.robot.getLeft()) + \
			"\nCenter: " + str(self.robot.getCenter()) + \
			"\nRight: " + str(self.robot.getRight()) + \
			"\nSensor margin: " + str(self.robot.getMargin()) + \
			"\nAlgorithm: " + str(self.robot.getAlgorithm()))
		self.after(100, self.updateData) # Request to be called again after 100 milliseconds

##
# Objects which holds the GUI
#
class Capp:

	##
	# Constructor
	# 
	# Parameters
	# master = root Tk object
	# robot = robot object
	#
	# Initiates the visible frame
	def __init__(self, master, robot):
	
		self.robot=robot # Save robot-object

		self.frame = Frame(master, width=400, height=400) # Initiate main frame
		self.frame.grid()
		master.title("FKUB")
		
		self.bRight = Button(self.frame, text="Right")  # Initiate "Right" button
		self.bRight.grid(column=2, row=1) 		# Where
		self.bRight.bind("<Button-1>", self.right)	# Set event-handler for left mouse-button down
		self.bRight.bind("<ButtonRelease-1>", self.up)	# Set event-handler for left mouse-button up

		self.bLeft = Button(self.frame, text="Left") 	# Again with "Left"
		self.bLeft.grid(column=0, row=1)
		self.bLeft.bind("<Button-1>", self.left)
		self.bLeft.bind("<ButtonRelease-1>", self.up)

		self.bForward = Button(self.frame, text="Forward") # ...
		self.bForward.grid(column=1, row=0)
		self.bForward.bind("<Button-1>", self.forward)
		self.bForward.bind("<ButtonRelease-1>", self.up)

		self.bBackward = Button(self.frame, text="Backward") # ...
		self.bBackward.grid(column=1, row=1)
		self.bBackward.bind("<Button-1>", self.backward)
		self.bBackward.bind("<ButtonRelease-1>", self.up)
		
		self.bLock = Button(self.frame, text="Start Program", command=self.startProgram) 	# More buttons, using the command parameter 
		self.bLock.grid(column=3, row=0)									# to set up event-handler, since we don't need to 
															# know if the button is held down
		self.bLock = Button(self.frame, text="Stop Program", command=self.robot.stopProgram)
		self.bLock.grid(column=3, row=1)
		
		self.bLock = Button(self.frame, text="LF On/Off", command=self.togglelock)
		self.bLock.grid(column=3, row=2)

		self.bnextLF = Button(self.frame, text="Change LF", command=self.nextLF)
		self.bnextLF.grid(column=3, row=3)

		self.bIncSpeed = Button(self.frame, text="Inc Speed", command=self.robot.incSpeed)
		self.bIncSpeed.grid(column=3, row=4)
		
		self.bDecSpeed = Button(self.frame, text="Dec Speed", command=self.robot.decSpeed)
		self.bDecSpeed.grid(column=3, row=5)

		self.bCalibrateLeft = Button(self.frame, text="Cal Left", command=self.calibrateLeft)
		self.bCalibrateLeft.grid(column=0, row=6)
		
		self.bCalibrateCenter = Button(self.frame, text="Cal Center", command=self.calibrateCenter)
		self.bCalibrateCenter.grid(column=1, row=6)
		
		self.bCalibrateRight = Button(self.frame, text="Cal Right", command=self.calibrateRight)
		self.bCalibrateRight.grid(column=2, row=6)

		self.bCalibrateSwitch = Button(self.frame, text="Switch", command=self.robot.calibrateSwitch)
		self.bCalibrateSwitch.grid(column=3, row=6)

		self.bSetSpeed = Button(self.frame, text="Set Speed", command=self.setSpeed)
		self.bSetSpeed.grid(column=3, row=7)

		self.bSetMargin = Button(self.frame, text="Set Margin", command=self.setMargin)
		self.bSetMargin.grid(column=0, row=7)

		self.input = StringVar()      # Text-field used to read input from user
		self.input.set("")
		self.eInput = Entry(self.frame, textvariable = self.input, bg="white")
		self.eInput.grid(column=1, row=7, columnspan=2, pady=10)

		self.sensordata = ThreadSafeLabel(self.frame, self.robot)			# Our custom label which should update itself
		self.sensordata.grid(column=0, columnspan=3, row=2, rowspan=4)
		
		
		self.lLeftWheel = Label(self.frame, text = "Inner Wheel:")
		self.lLeftWheel.grid(column=0, row=8, pady=0, padx=0)
		
		self.lLeftMult = Label(self.frame, text = "G:(left area)")
		self.lLeftMult.grid(column=0, row=9, pady=0, padx=0)
		
		self.leftMult = StringVar()
		self.leftMult.set("35")
		self.eLeftMult = Entry(self.frame, textvariable=self.leftMult, bg="white")
		self.eLeftMult.grid(column=1, row=9, pady=0, padx=0)
		
		self.lLeftConst = Label(self.frame, text = "H:(right area)")
		self.lLeftConst.grid(column=2, row=9, pady=0, padx=0)
		
		self.leftConst = StringVar()
		self.leftConst.set("60")
		self.eLeftConst = Entry(self.frame, textvariable=self.leftConst, bg="white")
		self.eLeftConst.grid(column=3, row=9, pady=0, padx=0)
		
		self.lLeftDiff = Label(self.frame, text = "I:(margin)")
		self.lLeftDiff.grid(column=0, row=10, pady=0, padx=0)
		
		self.leftDiff = StringVar()
		self.leftDiff.set("5")
		self.eLeftDiff = Entry(self.frame, textvariable=self.leftDiff, bg="white")
		self.eLeftDiff.grid(column=1, row=10, pady=0, padx=0)
		
		self.lRightWheel = Label(self.frame, text = "")
		self.lRightWheel.grid(column=0, row=11, pady=0, padx=0)
		
		self.lRightMult = Label(self.frame, text = "J:(speed1)")
		self.lRightMult.grid(column=0, row=12, pady=0, padx=0)
		
		self.rightMult = StringVar()
		self.rightMult.set("60")
		self.eRightMult = Entry(self.frame, textvariable=self.rightMult, bg="white")
		self.eRightMult.grid(column=1, row=12, pady=0, padx=0)
		
		self.lRightConst = Label(self.frame, text = "K:(speed2)")
		self.lRightConst.grid(column=2, row=12, pady=0, padx=0)
		
		self.rightConst = StringVar()
		self.rightConst.set("90")
		self.eRightConst = Entry(self.frame, textvariable=self.rightConst, bg="white")
		self.eRightConst.grid(column=3, row=12, pady=0, padx=0)
		
		self.lRightDiff = Label(self.frame, text = "L:(turn speed)")
		self.lRightDiff.grid(column=0, row=13, pady=0, padx=0)
		
		self.rightDiff = StringVar()
		self.rightDiff.set("40")
		self.eRightDiff = Entry(self.frame, textvariable=self.rightDiff, bg="white")
		self.eRightDiff.grid(column=1, row=13, pady=0, padx=0)
		
		self.bSetRightArea = Button(self.frame, text="Send", command=self.setVars)
		self.bSetRightArea.grid(column=2, row=14, pady=0, padx=0)

	def up(self, event):		# Some of event handlers.  
		self.robot.setButtonPressed(0)
		self.robot.stop()
	def right(self, event):
		self.robot.setButtonPressed(1)
		self.robot.right()
	def left(self, event):
		self.robot.setButtonPressed(1)
		self.robot.left()
	def forward(self, event):
		self.robot.setButtonPressed(1)
		self.robot.forward()
	def backward(self, event):
		self.robot.setButtonPressed(1)
		self.robot.backward()
	def calibrateLeft(self):
		setting = self.input.get()
		if setting.isdigit():
			self.robot.calibrateLeft(int(setting))
		else : self.robot.calibrateLeft()
	def calibrateCenter(self):
		setting = self.input.get()
		if setting.isdigit():
			self.robot.calibrateCenter(int(setting))
		else : self.robot.calibrateCenter()
	def calibrateRight(self):
		setting = self.input.get()
		if setting.isdigit():
			self.robot.calibrateRight(int(setting))
		else : self.robot.calibrateRight()
	def setSpeed(self):
		setting = self.input.get()
		if setting.isdigit() and int(setting) <= 100 and setting >= 0 :
			self.robot.setSpeed(int(setting))
	def setMargin(self):
		setting = self.input.get()
		if setting.isdigit():
			self.robot.setMargin(int(setting))
	def nextLF(self):
		setting = self.input.get()
		if setting.isdigit():
			self.robot.nextLF(int(setting))
		else : self.robot.nextLF()
	def startProgram(self):
		setting = self.input.get()
		if setting == "":
			self.robot.startProgram()
		else : self.robot.startProgram(setting)
	def togglelock(self):		# Sets or unsets btLock in the NXC code to stop the Line Following-algorithm
		if self.robot.getLock(): # btLock is set unset
			self.robot.setLock(0)
			if debug : print("Unlock") # Debug
		else: 			# Otherwise set
			self.robot.setLock(1)
			if debug : print("Lock")	# Debug
	def setVars(self):
		self.robot.setAlgo((int(self.leftMult.get()), int(self.leftConst.get()), int(self.leftDiff.get()), int(self.rightMult.get()), int(self.rightConst.get()), int(self.rightDiff.get())))
		return


##
# Class for handling communication between threads and NXT
# This class inherits from Thread to make the function run() run in a separate thread.
# Run() reads keyboard input and also reads messages from the NXT. 
# This makes it possible for the class to udate itself and we don't have to do through some event in Capp 
# Read from ThreadSafeLabel and outputted into the GUI
# Send commands to the NXT with nxt-object
class Crobot(threading.Thread):

	##
	# Constructor, initiates variables
	#
	# Parameter nxt = nxt-object that is connected to the NXT
	def __init__(self, nxt):
		
		threading.Thread.__init__(self)
		
		size = width, height =  100, 100			# Set up a very small pygame screen. This is necessary
		self.screen = pygame.display.set_mode(size) 	# since the event loop in pygame don't start without it
		pygame.display.set_caption("FKUB")
		
		self.nxt = nxt
		self.light = 0
		self.us = 0
		self.touch = 0
		self.btlock = 0		# NXT is locked, no Line Following-algorithm is running
		self.speed = 0
		self.left_area = 0	# Calibration data
		self.right_area = 0
		self.center_area = 0
		self.lastsent = 0	# Keeps tracks of last sent command to avoid sending commands when unnecessary
		self.button_pressed = 0 # Fix for not send stop while a button is pressed
		self.algorithm = 0
		self.margin = 0
		
	def run(self):	# Thread loop
		
		while self.nxt.error() != ERROR : # Run while we got a connection to the NXT
			
			pygame.event.pump()              # Let pygame process its eventqueue
			pressed_keys = pygame.key.get_pressed()		# Get a list with pressed keys
			if (pygame.key.get_focused()):			
				if pressed_keys[pygame.K_UP] and pressed_keys[pygame.K_RIGHT]: self.rightForw() # Check for pressed keys and run commands
				elif pressed_keys[pygame.K_UP] and pressed_keys[pygame.K_LEFT]: self.leftForw() # when needed
				elif pressed_keys[pygame.K_UP]: self.forward()
				elif pressed_keys[pygame.K_RIGHT]: self.right()
				elif pressed_keys[pygame.K_LEFT]: self.left()
				elif pressed_keys[pygame.K_DOWN]: self.backward()
				elif pressed_keys[pygame.K_SPACE] and self.getLock(): 
					self.setLock(0)
					time.sleep(0.05) 	# Sleep to not immediately read the same key again and run again
				elif pressed_keys[pygame.K_SPACE] and self.getLock() == 0: 
					self.setLock(1)
					time.sleep(0.05)
				elif pressed_keys[pygame.K_KP_PLUS]: 
					self.incSpeed()
					time.sleep(0.05)
				elif pressed_keys[pygame.K_KP_MINUS]: 
					self.decSpeed()
					time.sleep(0.05)
				elif not self.getButtonPressed(): self.stop()  # No buttons are pressed stop robot
			elif not self.getButtonPressed(): self.stop()
			
			if debug_nobluetooth: time.sleep(0.5) 	# Debug, runs to fast if not reading from bluetooth, easier to read messages if slow
			else:  time.sleep(0.01)			# No need to run faster than this

			message = self.nxt.readmessage() 		# Read message from NXT
			if message == EMPTY : continue		# Messagequeue is empty
			elif isinstance(message, str) : 	# If a string is returned
				if debug == 2 : 
					for i in range(0, len(message)): 
						print "R: ", hex(ord(message[i])), " ", # Print to console
					print 
				if len(message) == 10 :		# Right length of message
					if debug_nobluetooth: 	# Debug, write some data to see that everthing is update like it should
						self.light += 1
						self.us += 5
						self.speed += 10 
					else:
						self.light = ord(message[0]) 	# Convert recieved data to integer
						self.us = ord(message[1])		# and write to robot-object
						self.touch = ord(message[2])
						self.btlock = ord(message[3])
						self.speed = ord(message[4])
						self.left_area = ord(message[5])
						self.center_area = ord(message[6])
						self.right_area = ord(message[7])
						self.algorithm = ord(message[8])
						self.margin = ord(message[9])
										
					
			elif isinstance(message, int) : print "E: ", hex(message) # We recieved an integer, something is wrong, output to console
			else : print "Message.typ() == : ", type(message)

	##
	# Functions that returns data stored in this object.
	# Zero bytes is interpreted as termination-bytes in 
	# the bluetooth-protocol so they are sent as 0xff
	# and stored that way too.
	# These functions convert them back.
	def getLight(self): 
		if self.light != 0xff : return self.light
		else : return 0
	def getUs(self): 
		if self.us != 0xff : return self.us
		else : return 0
	def getTouch(self):
		if self.touch != 0xff : return self.touch
		else : return 0
	def getLock(self): 
		if self.btlock == 0xff : return 0
		else : return 1
	def getSpeed(self):
		if self.speed != 0xff : return self.speed
		else : return 0
	def getLeft(self):
		if self.left_area != 0xff : return self.left_area
		else : return 0
	def getCenter(self):
		if self.center_area != 0xff : return self.center_area
		else : return 0
	def getRight(self):
		if self.right_area != 0xff : return self.right_area
		else : return 0
	def getButtonPressed(self): return self.button_pressed
	def getMargin(self): 
		if self.margin != 0xff : return self.margin
		else : return 0
	def getAlgorithm(self):
		if self.algorithm != 0xff : return self.algorithm
		else : return 0
	##
	# Functions for controling the NXT
	def incSpeed(self): # Increases speed by 5
		self.nxt.sendmessage(INCSPEED)
	def decSpeed(self): # Decreases speed by 5
		self.nxt.sendmessage(DECSPEED)
	def setLock(self, set): # Turns the Line Following-algorithm on or off
		if set : 
			self.nxt.sendmessage(LOCK)
		else : 
			self.nxt.sendmessage(UNLOCK)
	##
	#Movement
	def right(self):
		if self.lastsent == RIGHT: return 	# If this was the last sent message don't send again
		self.lastsent = RIGHT		# Set lastsent to same unique constant as in above if-statement
		self.nxt.sendmessage(RIGHT)
	def rightForw(self):
		if self.lastsent == RIGHTFORW: return
		self.lastsent = RIGHTFORW
		self.nxt.sendmessage(RIGHTFORW)
	def forward(self):
		if self.lastsent == STRAIGHT: return
		self.lastsent = STRAIGHT
		self.nxt.sendmessage(STRAIGHT)
	def left(self):
		if self.lastsent == LEFT: return
		self.lastsent = LEFT
		self.nxt.sendmessage(LEFT)
	def leftForw(self):
		if self.lastsent == LEFTFORW: return
		self.lastsent = LEFTFORW
		self.nxt.sendmessage(LEFTFORW)
	def backward(self):
		if self.lastsent == BACKWARD: return
		self.lastsent = BACKWARD
		self.nxt.sendmessage(BACKWARD)
	def stop(self):
		if self.lastsent == STILL: return
		self.lastsent = STILL
		self.nxt.sendmessage(STILL)
	def setSpeed(self, value):
		self.nxt.sendmessage(SETSPEED + chr(value))
	##
	# Fix for not sending stop when a button is clicked
	def setButtonPressed(self, pressed):
		if pressed: self.button_pressed = 1
		else: self.button_pressed = 0
	##
	# Calibration
	def calibrateLeft(self, value=DEFAULT):
		self.nxt.sendmessage(CALIBRATELEFT + chr(value))
	def calibrateCenter(self, value=DEFAULT):
		self.nxt.sendmessage(CALIBRATECENTER + chr(value))
	def calibrateRight(self, value=DEFAULT):
		self.nxt.sendmessage(CALIBRATERIGHT + chr(value))
	def calibrateSwitch(self):
		self.nxt.sendmessage(CALIBRATELEFT + chr(self.right_area))
		self.nxt.sendmessage(CALIBRATERIGHT + chr(self.left_area))
	def nextLF(self, value=DEFAULT): 
		self.nxt.sendmessage(NEXTLF + chr(value))
	def setMargin(self, value):
		self.nxt.sendmessage(SETMARGIN + chr(value))
	def setAlgo(self, values):     # This function is used for storing data in the NXT 
		message = SETALGO         # Data sent with this function can be accessed in the NXT code
		for i in values :         # and used for, as an example, calibrate the LF-algorithm without reprogramming the NXT
			if i+50 == 0 : 
				message = message + chr(0xff)
			else: message = message + chr(i + 50) 
		self.nxt.sendmessage(message)
	##
	# Program control
	def startProgram(self, program=PROGRAM):
		if debug : print hex(ord(self.nxt.startprogram(program + ".rxe")))
		else : self.nxt.startprogram(program + ".rxe")
		
	def stopProgram(self):
		if debug : print hex(ord(self.nxt.stopprogram()))
		else : self.nxt.stopprogram()


nxt = Cnxt(NXT_ADRESS) 	# Create nxt-object

robot = Crobot(nxt) 	# Create robot-object

root = Tk()		# Start GUI
app = Capp(root, robot)

robot.start()       # Start thread in robot

pygame.init() 		# Initiate pygame

root.mainloop()		# Start mainloop of GUI

robot.setLock(0)	# Start Line Following-algorithm before quiting
nxt.shutdown()		# Shutdown bluetooth

time.sleep(1)		# Wait for some unexplained reason

