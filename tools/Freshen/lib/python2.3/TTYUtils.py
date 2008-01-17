import os, sys
class Screen:
	height,width=[int(x) for x in os.popen('stty size').readline().strip().split(' ')]
	colours = {'red': "\033[1;31m",
		'blue': "\033[1;34m",
		'green': "\033[0;32m",
		'yellow': "\033[1;33m",
		'brown': "\033[33m",
		'cyan': "\033[0;36m",
		'redbg': "\033[0;48m",
		'normal': "\033[0m"
	}

class ProgressBar:
	def __init__(self, size, size2=0):
		self.size = size
		self.size2 = size2
		self.value = 0
		self.value2 = 0
		self.points = -1
		self.points2 = -1
		if size2:
			self.barWidth = Screen.width/2 - 8
		else:
			self.barWidth = Screen.width - 8
		self.percent = 0
		self.percent2 = 0
		self.enabled = True
	def draw(self):
		if not self.enabled:
			return
		if self.size2:
			points = int(float(self.value)/self.size*self.barWidth)
			points2 = int(float(self.value2)/self.size2*self.barWidth)
			pc = int(100*float(self.value)/self.size)
			pc2 = int(100*float(self.value2)/self.size2)
			if points != self.points or pc != self.percent or points2 != self.points2 or pc2 != self.percent2:
				sys.stderr.write(" [%s%s] %3i%%"%("#"*points, " "*(self.barWidth-points), pc))
				sys.stderr.write(" [%s%s] %3i%%\015"%("#"*points2, " "*(self.barWidth-points2), pc2))
				sys.stderr.flush()
				self.points = points
				self.percent = pc
				self.points2 = points2
				self.percent2 = pc2
		else:
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
	def inc2(self,draw=True, n=1):
		self.value2+=n
		if draw:
			self.draw()
	def clear(self):
		sys.stderr.write(' '*Screen.width+"\015")
		self.enabled = False
