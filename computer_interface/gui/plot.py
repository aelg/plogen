#! /usr/bin/python
# coding: Latin1

##\ingroup computer_interface Datorgränsnitt
# @{


## @file
#  Hanterar plottning av data.


import matplotlib.pyplot as plt
import matplotlib.animation as animation
import time
import sys

class Plot():

  def __init__(self, robot):

    self.robot = robot
    self.t = 0

    self.fig = plt.figure();
    self.ax = self.fig.add_subplot(111)
    self.pLine, = self.ax.plot([], [], lw=2)
    self.dLine, = self.ax.plot([], [], lw=2)
    self.rightLine, = self.ax.plot([], [], lw=2)
    self.leftLine, = self.ax.plot([], [], lw=2)
    self.ax.set_ylim(-100, 240)
    self.ax.set_xlim(0, 5)
    self.ax.grid()
    self.xData, self.pYData, self.dYData, self.leftYData, self.rightYData = [], [], [], [], []

  def data_gen(self):
    t = self.t
    while True:
      t += 0.01
      yield t, self.robot.getRegP(), self.robot.getRegD(), self.robot.getIRShortRight(), self.robot.getIRShortLeft()

  def run(self, data):

    t,pY, dY, right, left = data
  
    self.xData.append(t)
    self.pYData.append(pY)
    self.dYData.append(dY)
    self.rightYData.append(right)
    self.leftYData.append(left)
    xmin, xmax = self.ax.get_xlim()
    if t >= xmax:
        self.ax.set_xlim(t-4, t+1)
        self.ax.figure.canvas.draw()
    self.pLine.set_data(self.xData, self.pYData)
    self.dLine.set_data(self.xData, self.dYData)
    self.rightLine.set_data(self.xData, self.rightYData)
    self.leftLine.set_data(self.xData, self.leftYData)

    return self.pLine, self.dLine, self.rightLine, self.leftLine

  def start(self):

    ani = animation.FuncAnimation(self.fig, self.run, self.data_gen, blit=True, interval=100,
      repeat=False, save_count=0)
    plt.show()

## @}
