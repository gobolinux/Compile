from qt import *
from GraphicalTailForm import *

class GraphicalTail :
	output = None
	w = None
	def __init__(self) :
		self.output = open('/tmp/GoboLinuxInstall.log', 'w')
		self.w = GraphicalTailForm()
		
		self.color = {}
		self.color['Gray']     =('\033[1;30m' , '<font color="#777777">')
		self.color['BoldBlue'] =('\033[1;34m' , '<font color="#777700">')
		self.color['Brown']    =('\033[33m'   , '<font color="#777700">')
		self.color['Yellow']   =('\033[1;33m' , '<font color="#777700">')
		self.color['BoldGreen']=('\033[1;32m' , '<font color="#005050">')
		self.color['BoldRed']  =('\033[1;31m' , '<font color="#FF0000">')
		self.color['Cyan']     =('\033[36m'   , '<font color="#005050">')
		self.color['BoldCyan'] =('\033[1;36m' , '<font color="#777700">')
		self.color['RedWhite'] =('\033[41;37m', '<font color="#777700">')
		self.color['Normal']   =('\033[0m'    , '</font>')#'"#000000"')

		#self.w.textWidget.setHScrollBarMode(QScrollView.AlwaysOff)
		#self.w.textWidget.setVScrollBarMode(QScrollView.AlwaysOff)

	def enableOk(self) :
		#self.w.textWidget.setHScrollBarMode(QScrollView.Auto)
		#self.w.textWidget.setVScrollBarMode(QScrollView.Auto)
		self.output.close()
		self.w.okButton.setEnabled(1)

	def append(self, s) :
		qApp.lock()
		vs = self.w.textWidget.verticalScrollBar()
		doScroll  = (vs.maxValue() <= vs.value())
		
		try :
			self.output.write(s)
		except :
			pass
		for key in self.color.keys() :
			terminal, html = self.color[key]
			s = s.replace(terminal, html)
		self.w.textWidget.append(s)
		
		if self.w.autoScroll.isChecked() :
			self.w.textWidget.ensureVisible (0, 999999) # scroll down
		
		qApp.unlock()
		#QApplication.
		#qApp.processEvents()
	
	
	def show(self) :
		self.w.show()
