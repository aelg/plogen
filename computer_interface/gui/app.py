from Tkinter import * 	# GUI
from thread_safe_label import ThreadSafeLabel
import threading 	# Threading
from constants import *
from plot import Plot
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

    self.bLock = Button(self.frame, text="Start Auto", command=self.setAuto)
    self.bLock.grid(column=3, row=0)

    self.bPlot = Button(self.frame, text="Start Reg Plot", command=self.startPlot)
    self.bPlot.grid(column=3, row=1)

    self.bChooseMode = Button(self.frame, text="Set Mode", command=self.setMode)
    self.bChooseMode.grid(column=3, row=2)

    self.lbMode = Listbox(self.frame, selectmode=SINGLE)
    self.lbMode.grid(column=3, row=4)
    self.lbMode.insert(END, "Manual")
    self.lbMode.insert(END, "Line Follow")
    self.lbMode.insert(END, "PD-reg")
    self.lbMode.insert(END, "Auto")

    self.input = StringVar()      # Text-field used to read input from user
    self.input.set("")
    self.eInput = Entry(self.frame, textvariable = self.input, bg="white")
    self.eInput.grid(column=1, row=7, columnspan=2, pady=10)

    self.sensordata = ThreadSafeLabel(self.frame, self.robot)			# Our custom label which should update itself
    self.sensordata.grid(column=0, columnspan=3, row=2, rowspan=4)


    self.lRegulator = Label(self.frame, text = "Regulator:")
    self.lRegulator.grid(column=0, row=8, pady=0, padx=0)

    self.lRegP = Label(self.frame, text = "P:")
    self.lRegP.grid(column=0, row=9, pady=0, padx=0)

    self.sRegP = StringVar()
    self.sRegP.set("35")
    self.eRegP = Entry(self.frame, textvariable=self.sRegP, bg="white")
    self.eRegP.grid(column=1, row=9, pady=0, padx=0)

    self.lSpeed = Label(self.frame, text = "Speed:")
    self.lSpeed.grid(column=2, row=9, pady=0, padx=0)

    self.sSpeed = StringVar()
    self.sSpeed.set("60")
    self.eSpeed = Entry(self.frame, textvariable=self.sSpeed, bg="white")
    self.eSpeed.grid(column=3, row=9, pady=0, padx=0)

    self.lRegD = Label(self.frame, text = "D:")
    self.lRegD.grid(column=0, row=10, pady=0, padx=0)

    self.sRegD = StringVar()
    self.sRegD.set("5")
    self.eRegD = Entry(self.frame, textvariable=self.sRegD, bg="white")
    self.eRegD.grid(column=1, row=10, pady=0, padx=0)

    self.bSendRegParams = Button(self.frame, text="Send", command=self.sendRegParams)
    self.bSendRegParams.grid(column=2, row=14, pady=0, padx=0)

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
  def setSpeed(self):
    setting = self.input.get()
    if setting.isdigit() and int(setting) <= 100 and setting >= 0 :
      self.robot.setSpeed(int(setting))
  def setAuto(self):
    self.robot.enableAuto()
  def startPlot(self):
    self.plot = Plot(self.robot)
    self.plot.start()
  def setMode(self):
    cur = self.lbMode.curselection()
    if cur == 0:
      self.robot.setMode(MODE_MANUAL)
    elif cur == 1:
      self.robot.setMode(MODE_LINE_FOLLOW)
    elif cur == 2:
      self.robot.setMode(MODE_PD)
    elif cur == 2:
      self.robot.setMode(MODE_AUTO)

  def sendRegParams(self):
    self.robot.sendRegParams(self.sRegP.get(), self.sRegD.get(), speed = self.sSpeed.get())

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
            if message[i] == IR_DIFF:
              self.robot.IRDiff = ord(message[i+1])
            if message[i] == IR_ANGLE:
              self.robot.IRAngle = ord(message[i+1])
            if message[i] == TAPE:
              self.robot.tape = ord(message[i+1])

        if message[0] == chr(CMD_REG_PARAMS):
          if not(len(message) % 3):
            print "Error illformed message from Plogen."
            continue
          for i in range(1, len(message)-1, 3):
            print i
            if message[i] == REG_P:
              self.robot.regP = (ord(message[i+1]) * 256) + ord(message[i+2])
              if self.robot.regP > (1 << 8) : 
                self.robot.regP -= (1 << 16) 
            if message[i] == REG_D:
              self.robot.regD = (ord(message[i+1]) * 256) + ord(message[i+2])
              if self.robot.regD > (1 << 8) : 
                self.robot.regD -= (1 << 16) 
            if message[i] == REG_SPEED:
              self.robot.speed = (ord(message[i+1]) * 256) + ord(message[i+2])
              if self.robot.speed > (1 << 8) : 
                self.robot.speed -= (1 << 16) 
          
      elif isinstance(message, int) : print "E: ", hex(message) # We recieved an integer, something is wrong, output to console
      else : print "Message.typ() == : ", type(message)
    print 'Exiting app thread!.'
