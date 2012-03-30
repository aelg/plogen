from Tkinter import * 	# GUI
from thread_safe_label import ThreadSafeLabel
import threading 	# Threading
from constants import *
##
# Objects which holds the GUI
#
class Capp(threading.Thread):

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

    threading.Thread.__init__(self)

    self.frame = Frame(master, width=400, height=400) # Initiate main frame
    self.frame.grid()
    master.title("Plogen")

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

    self.bLock = Button(self.frame, text="Start Program")#, command=self.startProgram) 	# More buttons, using the command parameter 
    self.bLock.grid(column=3, row=0)									# to set up event-handler, since we don't need to 
                              # know if the button is held down
    self.bLock = Button(self.frame, text="Stop Program") #, command=self.robot.stopProgram)
    self.bLock.grid(column=3, row=1)

    self.bLock = Button(self.frame, text="LF On/Off", command=self.setAuto)
    self.bLock.grid(column=3, row=2)

    self.bnextLF = Button(self.frame, text="Change LF", command=self.nextLF)
    self.bnextLF.grid(column=3, row=3)

    self.bIncSpeed = Button(self.frame, text="Inc Speed")#, command=self.robot.incSpeed)
    self.bIncSpeed.grid(column=3, row=4)

    self.bDecSpeed = Button(self.frame, text="Dec Speed")#, command=self.robot.decSpeed)
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
  """def togglelock(self):		# Sets or unsets btLock in the NXC code to stop the Line Following-algorithm
    if self.robot.getLock(): # btLock is set unset
      self.robot.setLock(0)
      if debug : print("Unlock") # Debug
      else: 			# Otherwise set
        self.robot.setLock(1)
      if debug : print("Lock")	# Debug"""
  def setAuto(self):
    self.robot.stop()
  def setVars(self):
    self.robot.setAlgo((int(self.leftMult.get()), int(self.leftConst.get()), int(self.leftDiff.get()), int(self.rightMult.get()), int(self.rightConst.get()), int(self.rightDiff.get())))
    return

  def run(self):

    while self.robot.bt.error() != ERROR :
      message = self.robot.bt.readcmd()
      if isinstance(message, str) : 	# If a string is returned
        if debug == 2 : 
          for i in range(0, len(message)): 
            print "R: ", hex(ord(message[i])), " ", # Print to console
        if message[0] == chr(CMD_SENSOR_DATA):
          if not(len(message) % 2) :
            print "Error illformed message from Plogen."
            continue
          for i in range(1, len(message)-1, 2):
            if message[i] == IR_LONG_LEFT:
              self.robot.IRLongLeft = ord(message[i+1])
            if message[i] == IR_LONG_RIGHT:
              self.robot.IRLongRight = ord(message[i+1])
            if message[i] == IR_SHORT_LEFT:
              self.robot.IRShortLeft = ord(message[i+1])
            if message[i] == IR_SHORT_RIGHT:
              self.robot.IRShortRight = ord(message[i+1])
            if message[i] == IR_SHORT_BACK:
              self.robot.IRShortBack = ord(message[i+1])
            if message[i] == TAPE:
              self.robot.tape = ord(message[i+1])
      elif isinstance(message, int) : print "E: ", hex(message) # We recieved an integer, something is wrong, output to console
      else : print "Message.typ() == : ", type(message)
    print 'Exiting app thread!.'
