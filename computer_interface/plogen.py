#!/usr/bin/python
# coding: Latin1
##\defgroup computer_interface Datorgränsnitt
# @{


## @file
# Huvudprogram för datorgränsnittet. Drar igång datorgränsnittet.
# För att köra detta behövs:
# - Python 2.6
# - Pygame
# - Pybluez
# - Matplotlib
# - Numpy


debug = 2                               # Turns on or off some printing to console. 1 = Print outgoing data, 2 = Print in and outgoing data

from Tkinter import * 	# GUI
import time
from gui import *

send_socket_lock = threading.Semaphore() 	# Semaphore to handle different threads using the bluetooth-socket
recv_socket_lock = threading.Semaphore() 	# Semaphore to handle different threads using the bluetooth-socket

bt = Cbt(BT_ADRESS, send_socket_lock = send_socket_lock, recv_socket_lock = recv_socket_lock) 	# Create bt-object

robot = Crobot(bt) 	# Create robot-object

root = Tk()		# Start GUI
app = Capp(root, robot)

robot.start()       # Start thread in robot
app.start()         # Start thread in app

pygame.init() 		# Initiate pygame

root.mainloop()		# Start mainloop of GUI

robot.stop()
bt.shutdown()		# Shutdown bluetooth

time.sleep(0.01)		# Wait for some unexplained reason

##@}
