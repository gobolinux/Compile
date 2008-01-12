import os, sys
class Screen:
	height,width=map(int,os.popen('stty size').readline().strip().split(' '))

class ProgressBar:
	def __init__(self, size, value=0):
		self.size = size
		self.value = value
		self.points = -1
		self.barWidth = Screen.width - 29
	def draw(self):
		points = int(float(self.value)/self.size*self.barWidth)
		if points != self.points:
			pc = int(100*float(self.value)/self.size)
			sys.stdout.write(" [%s%s] %4i%% %i/%i\015"%("#"*points, " "*(self.barWidth-points), pc, self.value, self.size))
			sys.stdout.flush()
			self.points = points
	def inc(self,draw=True, n=1):
		self.value+=n
		if draw:
			self.draw()
	def clear(self):
		sys.stdout.write(' '*Screen.width+"\015")
		print "Got up to %s/%s"%(self.value,self.size)
