# coding: Latin1
from Tkinter import * 	# GUI

##\ingroup computer_interface Datorgränsnitt
# @{


## @file
#  Hanterar gränsnittet så det visar data som skickas till den.


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
    self.data.set("Antal tejp: " + str(self.robot.getTape()) + \
        "\nTejp value: " + str(self.robot.getTapeValue()) + " No: " + str(self.robot.getNumTape()) + \
        "\nIR Long Left: " + str(self.robot.getIRLongLeft()) + \
        "\nIR Long Right: " + str(self.robot.getIRLongRight()) + \
        "\nIR Short Left: " + str(self.robot.getIRShortLeft()) + \
        "\nIR Short Right: " + str(self.robot.getIRShortRight()) + \
        "\nIR Short Back: " + str(self.robot.getIRShortBack()) + \
        "\nIR Angle: " + str(self.robot.getIRAngle()) + \
        "\nIR Diff: " + str(self.robot.getIRDiff()) + \
        "\nReg P: " + str(self.robot.getRegP()) + \
        "\nReg D: " + str(self.robot.getRegD()) + \
        "\nSpeed: " + str(self.robot.getSpeed()))
    self.after(100, self.updateData) # Request to be called again after 100 milliseconds
## @}
