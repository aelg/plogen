#! /usr/bin/python

import matplotlib.pyplot as plt
import matplotlib.animation as animation
import time
import sys

f = open('/dev/rfcomm0', 'r+')

s = 'AT\r'

while s != 'q':
  f.write(s)
  time.sleep(0.01)
  r = f.read();
  print r + '\n'
  s = raw_input()

