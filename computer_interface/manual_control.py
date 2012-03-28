#! /usr/bin/python

import matplotlib.pyplot as plt
import matplotlib.animation as animation
import time
import sys
from bluetooth import *

s = 'hej'
r = ''

STOP = '' + chr(9) + chr(1) + chr(0x10)
FORWARD = '' + chr(9) + chr(1) + chr(0x0c)
RIGHT = '' + chr(9) + chr(1) + chr(0x0b)
LEFT = '' + chr(9) + chr(1) + chr(0x0a)
REVERSE = '' + chr(9) + chr(1) + chr(0x0d)

addr = '00:06:66:03:A9:9C'

bt = BluetoothSocket( RFCOMM ) 	# Create socket
error = bt.connect((addr, 1)); 	# Connect, channel 1
bt.setblocking(1) 		# Activates blocking, which makes the socket wait for data if there is none

"""class test():
  def send(self, s):
    for c in s:
      print ord(c);

bt = test()"""

#bt.send(chr(2))

for i in range(0, 20):
  #print r + '\n'
  #print(s)
  try:
    bt.send(FORWARD)
    time.sleep(1)
    bt.send(STOP)
    time.sleep(1)
    print "ahsdkf"
#    bt.send(RIGHT)
#    time.sleep(1)
#    bt.send(STOP)
#    time.sleep(1)
#    bt.send(LEFT)
#    time.sleep(1)
#    bt.send(STOP)
#    time.sleep(1)
#    bt.send(REVERSE)
#    time.sleep(1)
#    bt.send(STOP)
#    time.sleep(1)
  except btcommon.BluetoothError:
    """asdf"""

