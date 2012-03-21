#! /usr/bin/python

import matplotlib.pyplot as plt
import matplotlib.animation as animation
import time
import sys
from bluetooth import *

#f = open('/dev/rfcomm0', 'r+b')

s = 'hej'
r = ''

addr = '00:06:66:03:A9:9C'

bt = BluetoothSocket( RFCOMM ) 	# Create socket
error = bt.connect((addr, 1)); 	# Connect, channel 1
bt.setblocking(1) 		# Activates blocking, which makes the socket wait for data if there is none

while s != 'q':
  #print r + '\n'
  #print(s)
  try:
    s = raw_input()
    bt.send(chr(2) + chr(len(s)) + s)
    #time.sleep(1)
    r = bt.recv(100);
    print r 
  except btcommon.BluetoothError:
    """ asdf"""

