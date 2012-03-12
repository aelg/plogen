#! /usr/bin/python

import matplotlib.pyplot as plt
import matplotlib.animation as animation
import time
import sys

f = open('/dev/ttyUSB0', 'rw')

def data_gen():
  t = data_gen.t
  while True:
    #time.sleep(0.01)
    #print 'A'
    t += 0.1
    #s = raw_input('')
    s = f.read()
    while len(s) == 0:
      s = f.read()
    sys.stderr.write(s+'\n')
    yield t, ord(s[0])

data_gen.t = 0

fig = plt.figure();
ax = fig.add_subplot(111)
line, = ax.plot([], [], lw=2)
ax.set_ylim(-100, 240)
ax.set_xlim(0, 5)
ax.grid()
xdata, ydata = [], []

def run(data):

  t,y = data
  
  xdata.append(t)
  ydata.append(y)
  xmin, xmax = ax.get_xlim()
  if t >= xmax:
      ax.set_xlim(t-4, t+1)
      ax.figure.canvas.draw()
  line.set_data(xdata, ydata)

  return line,

print 'a\n'
ani = animation.FuncAnimation(fig, run, data_gen, blit=True, interval=100,
    repeat=False, save_count=0)
plt.show()

