# -*- encoding iso-8859-1 -*-
import sys

#class AbsInterface :


#class AbsDialog :


class AbsWizard :
	def __init__(self, name) : 
		self.screens = []     

	def setValue(self, field, value) :
		for screen in self.screens :
			if screen.setValue(field, value) :
				return

	def getValue(self, field) :
		for screen in self.screens :
			v = screen.getValue(field)
			if v != None :
				return v
		return None

	def setEnabled(self, field, enabled) :
		for screen in self.screens :
			if screen.setEnabled(field, enabled) :
				return

	def isEnabled(self, field) :
		for screen in self.screens :
			v = screen.isEnabled(field)
			if v != None :
				return v
		return None
	
	def done(self) :
		pass


class AbsScreen :
	def __init__(self) :
		self.fields = {}   
	
	def setValue(self, field, value) :
		if self.fields.has_key(field) :
			self.fields[field].setValue(value)
			return 1
		else :
			return 0

	def getValue(self, field) :
		if self.fields.has_key(field) :
			return self.fields[field].getValue()
		else :
			return None

	def setEnabled(self, field, enabled) :
		if self.fields.has_key(field) :
			self.fields[field].setEnabled(enabled)
			return 1
		else :
			return 0

	def isEnabled(self, field) :
		if self.fields.has_key(field) :
			return self.fields[field].isEnabled()
		else :
			return None


class AbsTranslator :
	doc = None
	lang = None
	
	def __init__(self, translationsFile = '', lang = '') :
		import libxml2, os
		if translationsFile and lang and os.access(translationsFile, os.R_OK):
			self.doc = libxml2.parseFile(translationsFile)
			self.lang = lang

	def tr(self,s) :
		if self.lang and self.doc :
			rs = s.replace("'",'&apos;').replace("\\", '&quot')
			results = self.doc.xpathEval('/TS/context/message[source = \'%s\']/translation/text()'%(rs,))
			if not results :
			    return s
			return results[0].content.replace('&apos;',"'").replace('&quot',"\\").decode('utf-8').encode('iso-8859-1')
		else :
			return s
