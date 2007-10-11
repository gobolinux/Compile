from wizard import *
from qt import *
import sys
from qttable import *

class NewQWizard(QWizard):
	def __init__(self, qabswizard):
		QWizard.__init__(self)
		self.qabswizard = qabswizard

	def currentScreen(self):
		i = self.indexOf(self.currentPage())
		return self.qabswizard.screens[i]


	def next(self):
		screen = self.currentScreen()
		if not screen.nextCB or screen.nextCB():
			QWizard.next(self)

	def finish(self):
		screen = self.currentScreen()
		if not screen.nextCB or screen.nextCB():
			QWizard.finish(self)

class AbsQtWizard(AbsWizard):
	def __init__(self, name):
		AbsWizard.__init__(self, name)
		self.app = QApplication([])
		self.qwizard = NewQWizard(self)
		self.qwizard.setCaption(name)
		self.lastScreen = None
		self.qwizard.setGeometry(QRect(50, 50, 480, 400))
		self.messageBoxPending = 0

	def showMessageBox(self, message, buttons = ['Ok, Cancel']):
		if self.messageBoxPending:
			return ''
		self.messageBoxPending = 1
		if len(buttons) == 1:
			i = QMessageBox.warning(self.qwizard, str(self.qwizard.caption())+' warning', message, buttons[0])
		elif len(buttons) == 2:
			i = QMessageBox.warning(self.qwizard, str(self.qwizard.caption())+' warning', message, buttons[0], buttons[1])
		if len(buttons) >= 3:
			i = QMessageBox.warning(self.qwizard, str(self.qwizard.caption())+' warning', message, buttons[0], buttons[1], buttons[2])
		self.messageBoxPending = 0
		return buttons[i]

	def start(self):
		self.app.setMainWidget(self.qwizard)
		self.qwizard.show()
		return self.app.exec_loop()

	def addScreen(self, absQtScreen, pos=0):
		# Add the screen at the end
		if pos == 0:
			self.screens.append(absQtScreen)
			self.qwizard.addPage(absQtScreen.widget, absQtScreen.widget.caption())
			if self.lastScreen:
				#more than one page yet
				self.qwizard.setFinishEnabled(self.lastScreen.widget, 0)
				self.qwizard.finishButton().setText("&Finish")
				self.qwizard.backButton().show()
			else:
				#just one page yet
				self.qwizard.finishButton().setText("&Ok")
				self.qwizard.backButton().hide()

			self.qwizard.helpButton().hide()
			self.lastScreen = absQtScreen
			self.qwizard.setFinishEnabled(self.lastScreen.widget, 1)
		# Add the screen right after the current one
		elif pos == -1:
			pos = self.qwizard.indexOf(self.qwizard.currentPage()) + 1
			self.screens.insert(pos, absQtScreen)
			self.qwizard.insertPage(absQtScreen.widget, absQtScreen.widget.caption(), pos)
		# Add the screen at a specific possition
		else:
			self.screens.insert(pos,absQtScreen)
			self.qwizard.insertPage(absQtScreen.widget, absQtScreen.widget.caption(),pos)
		self.qwizard.connect(self.qwizard.finishButton(), SIGNAL('released()'), self.__finish)
		self.qwizard.connect(self.qwizard.cancelButton(), SIGNAL('released()'), self.__cancel)

	def removeScreen(self, absQtScreen):
		self.screens.remove(absQtScreen)
		self.qwizard.removePage(absQtScreen.widget)

	def clear(self, newName = ''):
		lastName = self.qwizard.caption()
		if not newName:
			newName = lastName
		self.qwizard = QWizard()
		self.qwizard.setCaption(newName)
		self.lastScreen = None
		self.screens = []
		self.qwizard.setGeometry(QRect(50, 50, 480, 400))

	def __finish(self):
		self.app.exit(1)

	def __cancel(self):
		self.app.exit(0)

class AbsQtScreen(AbsScreen):
	def __init__(self, title = "I DON'T HAVE A NAME"):
		AbsScreen.__init__(self)
		self.widget = QWidget()
		self.pageLayout = QGridLayout(self.widget, 1, 1, 11, 6, "pageLayout")
		spacer = QSpacerItem(2, 2, QSizePolicy.Minimum,QSizePolicy.Minimum)
		#spacer.setSizeType(QSpacerItem.Minimum)
		self.pageLayout.addItem(spacer, 10, 0)
		self.rowsCount = 0
		self.setTitle(title)
		self.fieldsTypes = {}

		self.nextCB = None

	def __registerField(self, name, widget, fieldType):
		self.fields[name] = widget
		self.fieldsTypes[name] = fieldType

	def onValidate(self, nextCB):
		self.nextCB = nextCB

	def addImage(self, fileName):
		p = QPixmap()
		p.load(fileName)
		w = QLabel(self.widget,"")
		w.setPixmap(p)
		self.__addWidget(w)


	def setValue(self, fieldName, newValue):
		if self.fields.has_key(fieldName):
			field = self.fields[fieldName]
			fieldType = self.fieldsTypes[fieldName]

			if fieldType == 'QCheckBox':
				field.setChecked(newValue)
			elif fieldType == 'QListBox':
				import types
				if type(newValue) == types.TupleType:
					items, defaultValue = newValue
					field.clear()
					for item in items:
						field.insertItem(item)
					try:
						selectedIndex = items.index(defaultValue)
					except:
						selectedIndex = 0

					field.setSelected(selectedIndex, 1)
				else:
					try:
						selectedIndex = items.index(newValue)
						field.setSelected(selectedIndex, 1)
					except:
						pass
			elif fieldType == 'QButtonGroup':
				pass
			elif fieldType == 'QLineEdit':
				field.setText(newValue)
			elif fieldType == 'QLabel':
				field.setText(newValue)
			elif fieldType == 'QTable':
				import types
				if type(newValue) == types.ListType:
					for i in range(field.numRows()):
						field.item(i, 0).setChecked(str(field.item(i, 0).text()) in newValue)
				else:
					if len(newValue) == 1:
						defaultValue = newValue[0]
						items = self.getValue(fieldName)[0]
						for i in range(field.numRows()):
							field.item(i, 0).setChecked(str(field.item(i, 0).text()) in newValue)
					else:
						items, defaultValue = newValue
						field.setNumRows(len(items))
						j = 0
						for item in items:
							c = QCheckTableItem(field, item)
							c.setChecked(item in defaultValue)
							field.setItem(j, 0, c)
							j = j + 1
			else:
				return 0
			return 1
		else:
			return 0

	def getValue(self, fieldName):
		if self.fields.has_key(fieldName):
			field = self.fields[fieldName]
			fieldType = self.fieldsTypes[fieldName]

			if fieldType == 'QCheckBox':
				return int(field.isChecked())
			elif fieldType == 'QListBox':
				ret1 = []
				for i in range(field.count()):
					ret1.append(str(field.item(i).text()))
				if field.selectedItem():
					ret2 = str(field.selectedItem().text())
				else:
					ret2 = ''
				return (ret1,ret2)

			elif fieldType == 'QButtonGroup':
				ret = []
				for i in range(field.count()):
					ret.append(str(field.find(i).text()))
				return (ret, str(field.selected().text()))
			elif fieldType == 'QLineEdit':
				return unicode(field.text())
			elif fieldType == 'QTable':
				field.numRows()
				ret1 = []
				ret2 = []
				for i in range(field.numRows()):
					ret1.append(str(field.text(i, 0)))
					if field.item(i, 0).isChecked():
						ret2.append(str(field.text(i, 0)))
				return (ret1,ret2)
			else:
				return None

		else:
			#!has_key...
			return None


	def __addLayout(self, widget, row=-1, column=-1):
		#TODO:
		#pageLayout_2.addMultiCellWidget(self.textEdit2,1,1,0,1)

		if row  == -1 or column == -1:
			row = self.rowsCount
			column = 0
		self.pageLayout.addLayout(widget, row, column)

		if row >= self.rowsCount:
			self.rowsCount = row + 1


	def __addWidget(self, widget, row=-1, column=-1):
		#TODO:
		#pageLayout_2.addMultiCellWidget(self.textEdit2,1,1,0,1)

		if row  == -1:
			row = self.rowsCount

		if column == -1:
			column = 0

		self.pageLayout.addWidget(widget, row, column)

		if row >= self.rowsCount:
			self.rowsCount = row + 1


	def setTitle(self,title):
		self.widget.setCaption(title)

	def addBoolean(self, fieldName, label='', defaultValue=0, toolTip='', callBack=None):
		w = QCheckBox(self.widget, "w")
		w.setText(label)
		w.setChecked(defaultValue)
		if toolTip:
			QToolTip.add(w, toolTip)
		if fieldName:
			self.__registerField(fieldName, w, 'QCheckBox')
		if callBack:
			w.connect(w, SIGNAL('stateChanged(int)'), callBack)
		self.__addWidget(w)

	def addPassword(self, fieldName,label='', defaultValue='', toolTip='', callBack=None):
		w = QLineEdit(self.widget, "w")
		w.setText(defaultValue)
		w.setEchoMode(QLineEdit.Password)

		if callBack:
			w.connect(w, SIGNAL('lostFocus()'), callBack)

		if toolTip:
			QToolTip.add(w, toolTip)

		if fieldName:
			self.__registerField(fieldName, w, 'QLineEdit')

		if label:
			layout = QHBoxLayout(None, 0, 6, "w")

			l = QLabel(self.widget,"w")
			l.setText(label)
			layout.addWidget(l)
			layout.addWidget(w)

			self.__addLayout(layout)
		else:
			self.__addWidget(w)

	def addLineEdit(self, fieldName, label='', defaultValue='', toolTip='', callBack=None):
		w = QLineEdit(self.widget, "w")
		w.setText(defaultValue)

		if callBack:
			#w.connect(w, SIGNAL('lostFocus()'), callBack)
			w.connect(w, SIGNAL('textChanged( const QString & )'), callBack)

		if toolTip:
			QToolTip.add(w, toolTip)

		if fieldName:
			self.__registerField(fieldName, w, 'QLineEdit')

		if label:
			layout = QHBoxLayout(None, 0, 6, "w")

			l = QLabel(self.widget, "w")
			l.setText(label)
			layout.addWidget(l)
			layout.addWidget(w)

			self.__addLayout(layout)
		else:
			self.__addWidget(w)


	def addMultiLineEdit(self, fieldName='', label='', defaultValue='', toolTip='', callBack=None):
		w = QGroupBox(self.widget, "groupBox" + fieldName)
		w.setColumnLayout(0, Qt.Vertical)
		w.setTitle(label)

		gbLayout = QGridLayout(w.layout())
		gbLayout.setAlignment(Qt.AlignTop)

		mle = QMultiLineEdit(w, "w")
		mle.setText(defaultValue)
		mle.setReadOnly(not fieldName)

		gbLayout.addWidget(mle, 0, 0)

		if callBack:
			mle.connect(mle, SIGNAL('textChanged()'), callBack)

		if toolTip:
			QToolTip.add(w, toolTip)
		if fieldName:
			self.__registerField(fieldName, mle, 'QMultiLineEdit')

		self.__addWidget(w)
		return


	def addLabel(self, fieldName, label='', defaultValue='', toolTip=''):
		w = QLabel(self.widget, "")
		w.setText(label)
		if toolTip:
			QToolTip.add(w, toolTip)
		if fieldName:
			self.__registerField(fieldName, w, 'QLabel')
		self.__addWidget(w)

	def addButton(self, fieldName, label='', defaultValue='', toolTip='', callBack=None):
		w = QPushButton(self.widget,"")
		w.setText(label)
		w.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
		if toolTip:
			QToolTip.add(w, toolTip)
		if callBack:
			w.connect(w, SIGNAL("released()"), callBack)

		self.__addWidget(w)

	def addList(self, fieldName, label='', defaultValueTuple=([],''), toolTip='', callBack=None):
		items, defaultValue = defaultValueTuple
		if len(items)<= 5 and len(items) != 0:
			self.addRadioList(fieldName, label, defaultValueTuple, toolTip, callBack)
		else:
			self.addBoxList(fieldName, label, defaultValueTuple, toolTip, callBack)


	def addBoxList(self, fieldName, label='', defaultValueTuple=([],''), toolTip='', callBack=None):
		items, defaultValue = defaultValueTuple
		w = QGroupBox(self.widget,"w")
		w.setColumnLayout(0, Qt.Vertical)
		w.setTitle(label)
		if toolTip:
			QToolTip.add(w, toolTip)
		gbLayout = QGridLayout(w.layout())
		gbLayout.setAlignment(Qt.AlignTop)
		if callBack:
			w.connect(w, SIGNAL("highlighted (int)"), callBack)
		lb = QListBox(w, "w")
		for item in items:
			lb.insertItem(item)
		try:
			selectedIndex = items.index(defaultValue)
		except:
			selectedIndex = 0
		lb.setSelected(selectedIndex, 1)
		if fieldName:
			self.__registerField(fieldName, lb, 'QListBox')

		gbLayout.addWidget(lb, 0, 0)
		self.__addWidget(w)
		return

	def addRadioList(self, fieldName, label='', defaultValueTuple=([],''), toolTip='', callBack=None):
		items, defaultValue = defaultValueTuple
		w = QButtonGroup(self.widget,"w")
		w.setTitle(label)
		w.setColumnLayout(0, Qt.Vertical)

		if toolTip:
			QToolTip.add(w, toolTip)

		bgLayout = QGridLayout(w.layout())
		bgLayout.setAlignment(Qt.AlignTop)
		try:
			selectedIndex = items.index(defaultValue)
		except:
			selectedIndex = 0
		i=0
		for item in items:
			rb = QRadioButton(w, "w")
			if i == selectedIndex:
				rb.setChecked(1)
			rb.setText(item)
			bgLayout.addWidget(rb, i, 0)
			i=i+1

		if fieldName:
			self.__registerField(fieldName, w, 'QButtonGroup')
		if callBack:
			w.connect(w, SIGNAL("released(int)"), callBack)

		self.__addWidget(w)

	def __createGroupBoxAndLayout(self, label):
		gb = QGroupBox(self.widget,"w")
		gb.setColumnLayout(0, Qt.Vertical)
		gb.setTitle(label)
		gbLayout = QGridLayout(gb.layout())
		gbLayout.setAlignment(Qt.AlignTop)
		return gb, gbLayout

	def addCheckList(self,fieldName, label, defaultValueTuple=([],[]), toolTip='', callBack=None):
		items, defaultValue = defaultValueTuple
		b, l = self.__createGroupBoxAndLayout(label)

		w= AbsQtQTable(b, callBack)
		w.setNumCols(w.numCols() + 1)
		w.setShowGrid(0)
		w.setColumnWidth(0, 800)
		w.setLeftMargin(0)
		w.setTopMargin(0)

		if callBack:
			w.connect(w,SIGNAL("currentChanged(int,int)"), callBack)
			#w.connect(w,SIGNAL("clicked ( int , int , int , const QPoint &)"),callBack)
			#w.connect(w,SIGNAL("pressed( int , int , int , const QPoint &)"),callBack)

		if toolTip:
			QToolTip.add(w, toolTip)

		w.setNumRows(len(items))
		j = 0
		for item in items:
			c = QCheckTableItem(w, item)
			c.setChecked(item in defaultValue)
			w.setItem(j, 0, c)
			j = j+1

		l.addWidget(w, 0, 0)
		if fieldName:
			self.__registerField(fieldName, w, 'QTable')

		self.__addWidget(b)

#in the soon future, maybe all widgets will be extended
class AbsQtQTable(QTable):
	def __init__(self, w, callBack):
		QTable.__init__(self, w,  'w')
		self.callBack = callBack

	def contentsMouseReleaseEvent(self, e):
		QTable.contentsMouseReleaseEvent(self, e)
		if self.callBack:
			self.callBack()

	def keyPressEvent(self, e):
		QTable.keyPressEvent(self, e)
		if self.callBack:
			self.callBack()
