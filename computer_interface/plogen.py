#!/usr/bin/python

debug = 2                               # Turns on or off some printing to console. 1 = Print outgoing data, 2 = Print in and outgoing data

from Tkinter import * 	# GUI
import time
from gui import *

socket_lock = threading.Semaphore() 	# Semaphore to handle different threads using the bluetooth-socket

bt = Cbt(BT_ADRESS, socket_lock) 	# Create bt-object

robot = Crobot(bt) 	# Create robot-object

root = Tk()		# Start GUI
app = Capp(root, robot)

robot.start()       # Start thread in robot
app.start()         # Start thread in app

pygame.init() 		# Initiate pygame

root.mainloop()		# Start mainloop of GUI

robot.stop()
app.setAuto()	# Start Line Following-algorithm before quiting
bt.shutdown()		# Shutdown bluetooth

time.sleep(1)		# Wait for some unexplained reason

