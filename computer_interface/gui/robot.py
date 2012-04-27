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
    self.tape = 0
    self.numTape = 0
    self.tapeValue = 0
    self.speed = 0
    self.IRLongLeft = 0
    self.IRLongRight = 0
    self.IRShortLeft = 0
    self.IRShortRight = 0
    self.IRShortBack = 0
    self.IRDiff = 0
    self.IRAngle = 0
    self.regP = 0;
    self.regD = 0;
    self.speed = 0;
    self.autoMode = 0
    self.lastsent = 0	# Keeps tracks of last sent command to avoid sending commands when unnecessary
    self.button_pressed = 0 # Fix for not send stop while a button is pressed

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
      
  ##
  # Functions that returns data stored in this object.
  def getIRLongLeft(self):
    return self.IRLongLeft
  def getIRLongRight(self):
    return self.IRLongRight
  def getIRShortLeft(self):
    return self.IRShortLeft
  def getIRShortRight(self):
    return self.IRShortRight
  def getIRShortBack(self):
    return self.IRShortBack
  def getIRDiff(self):
    return self.IRDiff
  def getIRAngle(self):
    return self.IRAngle
  def getRegP(self):
    return self.regP
  def getRegD(self):
    return self.regD
  def getSpeed(self):
    return self.speed
  def getAutoMode(self):
    return self.autoMode
  def getTape(self):
    return self.tape
  def getNumTape(self):
    return self.numTape
  def getTapeValue(self):
    return self.tapeValue
  def getButtonPressed(self): return self.button_pressed
  ##
  # Functions for controling the BT
  #Movement
  def right(self):
    if self.lastsent == RIGHT: return 	# If this was the last sent message don't send again
    self.lastsent = RIGHT		# Set lastsent to same unique constant as in above if-statement
    self.bt.sendcmd(CMD_MANUAL, RIGHT)
  def rightForw(self):
    if self.lastsent == RIGHTFORW: return
    self.lastsent = RIGHTFORW
    self.bt.sendcmd(CMD_MANUAL, RIGHTFORW)
  def forward(self):
    if self.lastsent == FORWARD: return
    self.lastsent = FORWARD
    self.bt.sendcmd(CMD_MANUAL, FORWARD)
  def left(self):
    if self.lastsent == LEFT: return
    self.lastsent = LEFT
    self.bt.sendcmd(CMD_MANUAL, LEFT)
  def leftForw(self):
    if self.lastsent == LEFTFORW: return
    self.lastsent = LEFTFORW
    self.bt.sendcmd(CMD_MANUAL, LEFTFORW)
  def backward(self):
    if self.lastsent == BACKWARD: return
    self.lastsent = BACKWARD
    self.bt.sendcmd(CMD_MANUAL, BACKWARD)
  def stop(self):
    if self.lastsent == STOP: return
    self.lastsent = STOP
    self.bt.sendcmd(CMD_MANUAL, STOP)
  def setSpeed(self, value):
    self.bt.sendcmd(SETSPEED + chr(value))
  ##
  # Fix for not sending stop when a button is clicked
  def setButtonPressed(self, pressed):
    if pressed: self.button_pressed = 1
    else: self.button_pressed = 0

  ##
  # Turn on autonomous mode.
  def enableAuto(self):
    self.bt.sendcmd(CMD_AUTO_ON, '')

  ##
  # Send Regulator Params
  def sendRegParams(self, p, d, speed, timer):
    self.bt.sendcmd(CMD_SET_REG_PARAMS, REG_P + chr(int(p)) + REG_D + chr(int(d)) + REG_SPEED + chr(int(speed)) + REG_TIMER + chr(int(timer)))

  def setMode(self, mode):
    self.bt.sendcmd(CMD_SET_MODE, '' + chr(mode))

