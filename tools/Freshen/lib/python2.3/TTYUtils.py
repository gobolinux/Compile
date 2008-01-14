import os, sys
class Screen:
	height,width=map(int,os.popen('stty size').readline().strip().split(' '))
	colours = {'red': "\033[1;31m",
			'blue': "\033[1;34m",
			'green': "\033[0;32m",
			'yellow': "\033[1;33m",
			'brown': "\033[33m",
			'cyan': "\033[0;36m",
			'normal': "\033[0m"
		}

class ProgressBar:
	def __init__(self, size, value=0):
		self.size = size
		self.value = value
		self.points = -1
		self.barWidth = Screen.width - 8
		self.percent = 0
		self.draw()
	def draw(self):
		points = int(float(self.value)/self.size*self.barWidth)
		pc = int(100*float(self.value)/self.size)
		if points != self.points or pc != self.percent:
			sys.stderr.write(" [%s%s] %3i%%\015"%("#"*points, " "*(self.barWidth-points), pc))
			sys.stderr.flush()
			self.points = points
			self.percent = pc
	def inc(self,draw=True, n=1):
		self.value+=n
		if draw:
			self.draw()
	def clear(self):
		sys.stderr.write(' '*Screen.width+"\015")
