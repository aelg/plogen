#! /usr/bin/python

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
    self.ax.set_ylim(-100, 240)
    self.ax.set_xlim(0, 5)
    self.ax.grid()
    self.xData, self.pYData, self.dYData = [], [], []

  def data_gen(self):
    t = self.t
    while True:
      t += 0.01
      yield t, self.robot.getRegP(), self.robot.getRegD()

  def run(self, data):

    t,pY, dY = data
  
    self.xData.append(t)
    self.pYData.append(pY)
    self.dYData.append(dY)
    xmin, xmax = self.ax.get_xlim()
    if t >= xmax:
        self.ax.set_xlim(t-4, t+1)
        self.ax.figure.canvas.draw()
    self.pLine.set_data(self.xData, self.pYData)
    self.dLine.set_data(self.xData, self.dYData)

    return self.pLine, self.dLine

  def start(self):

    ani = animation.FuncAnimation(self.fig, self.run, self.data_gen, blit=True, interval=100,
      repeat=False, save_count=0)
    plt.show()

