import threading 	# Threading
import pygame  		# For keyboard input
import time
from constants import *

DEFAULT = 0xff
##
# Class for handling communication between threads and BT
# This class inherits from Thread to make the function run() run in a separate thread.
# Run() reads keyboard input and also reads messages from the BT. 
# This makes it possible for the class to udate itself and we don't have to do through some event in Capp 
# Read from ThreadSafeLabel and outputted into the GUI
# Send commands to the BT with bt-object
class Crobot(threading.Thread):

  ##
  # Constructor, initiates variables
  #
  # Parameter bt = bt-object that is connected to the BT
  def __init__(self, bt):

    threading.Thread.__init__(self)

    size = width, height =  100, 100			# Set up a very small pygame screen. This is necessary
    self.screen = pygame.display.set_mode(size) 	# since the event loop in pygame don't start without it
    pygame.display.set_caption("Plogen")

    self.bt = bt
    self.light = 0
    self.us = 0
    self.touch = 0
    self.btlock = 0		# BT is locked, no Line Following-algorithm is running
    self.speed = 0
    self.left_area = 0	# Calibration data
    self.right_area = 0
    self.center_area = 0
    self.lastsent = 0	# Keeps tracks of last sent command to avoid sending commands when unnecessary
    self.button_pressed = 0 # Fix for not send stop while a button is pressed
    self.algorithm = 0
    self.margin = 0

  def run(self):	# Thread loop

    while self.bt.error() != ERROR : # Run while we got a connection to the BT

      pygame.event.pump()              # Let pygame process its eventqueue
      pressed_keys = pygame.key.get_pressed()		# Get a list with pressed keys
      if (pygame.key.get_focused()):			
        if pressed_keys[pygame.K_UP] and pressed_keys[pygame.K_RIGHT]: self.rightForw() # Check for pressed keys and run commands
        elif pressed_keys[pygame.K_UP] and pressed_keys[pygame.K_LEFT]:
          self.leftForw() # when needed
        elif pressed_keys[pygame.K_UP]:
          self.forward()
        elif pressed_keys[pygame.K_RIGHT]:
          self.right()
        elif pressed_keys[pygame.K_LEFT]:
          self.left()
        elif pressed_keys[pygame.K_DOWN]:
          self.backward()
        elif pressed_keys[pygame.K_SPACE] and self.getLock(): 
          self.setAuto()
          time.sleep(0.05) 	# Sleep to not immediately read the same key again and run again
        elif pressed_keys[pygame.K_SPACE] and self.getLock() == 0: 
          self.setAuto()
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

      message = self.bt.readcmd() 		# Read message from BT
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
  # Functions for controling the BT
  def incSpeed(self): # Increases speed by 5
    self.bt.sendcmd(INCSPEED)
  def decSpeed(self): # Decreases speed by 5
    self.bt.sendcmd(DECSPEED)
  """def setLock(self, set): # Turns the Line Following-algorithm on or off
    if set : 
      self.bt.sendcmd(MODE)
    else : 
      self.bt.sendcmd(UNLOCK)""" 
  ##
  #Movement
  def right(self):
    if self.lastsent == RIGHT: return 	# If this was the last sent message don't send again
    self.lastsent = RIGHT		# Set lastsent to same unique constant as in above if-statement
    self.bt.sendcmd(MANUAL, RIGHT)
  def rightForw(self):
    if self.lastsent == RIGHTFORW: return
    self.lastsent = RIGHTFORW
    self.bt.sendcmd(MANUAL, RIGHTFORW)
  def forward(self):
    if self.lastsent == STRAIGHT: return
    self.lastsent = STRAIGHT
    self.bt.sendcmd(MANUAL, STRAIGHT)
  def left(self):
    if self.lastsent == LEFT: return
    self.lastsent = LEFT
    self.bt.sendcmd(MANUAL, LEFT)
  def leftForw(self):
    if self.lastsent == LEFTFORW: return
    self.lastsent = LEFTFORW
    self.bt.sendcmd(MANUAL, LEFTFORW)
  def backward(self):
    if self.lastsent == BACKWARD: return
    self.lastsent = BACKWARD
    self.bt.sendcmd(MANUAL, BACKWARD)
  def stop(self):
    if self.lastsent == STOP: return
    self.lastsent = STOP
    self.bt.sendcmd(MANUAL, STOP)
  def setSpeed(self, value):
    self.bt.sendcmd(SETSPEED + chr(value))
  ##
  # Fix for not sending stop when a button is clicked
  def setButtonPressed(self, pressed):
    if pressed: self.button_pressed = 1
    else: self.button_pressed = 0
  ##
  # Calibration
  def calibrateLeft(self, value=DEFAULT):
    self.bt.sendcmd(CALIBRATELEFT + chr(value))
  def calibrateCenter(self, value=DEFAULT):
    self.bt.sendcmd(CALIBRATECENTER + chr(value))
  def calibrateRight(self, value=DEFAULT):
    self.bt.sendcmd(CALIBRATERIGHT + chr(value))
  def calibrateSwitch(self):
    self.bt.sendcmd(CALIBRATELEFT + chr(self.right_area))
    self.bt.sendcmd(CALIBRATERIGHT + chr(self.left_area))
  def nextLF(self, value=DEFAULT): 
    self.bt.sendcmd(NEXTLF + chr(value))
  def setMargin(self, value):
    self.bt.sendcmd(SETMARGIN + chr(value))
  def setAlgo(self, values):     # This function is used for storing data in the BT 
    message = SETALGO         # Data sent with this function can be accessed in the BT code
    for i in values :         # and used for, as an example, calibrate the LF-algorithm without reprogramming the BT
      if i+50 == 0 : 
        message = message + chr(0xff)
      else: message = message + chr(i + 50) 
    self.bt.sendcmd(message)
