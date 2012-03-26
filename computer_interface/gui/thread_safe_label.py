from Tkinter import * 	# GUI

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
