# -*- encoding iso-8859-1 -*-
#!/usr/bin/python

import gtk
from wizard import *

class AbsGtkWizard(AbsWizard) :

   def __init__(self, name = '') :
      self.main_window = gtk.Window()
      self.main_window.set_title(name)
      self.vbox = gtk.VBox()
      self.main_window.add(self.vbox)

   def do_prev(self, w):
      #TODO: fix addScreen and then here remove current
      # screen widget and add the correct one 
      pass

   def do_next(self, w):
      #TODO: similar to do_prev
      pass

   def start(self) :
      self.main_window.show_all()
      self.main_window.connect("destroy", gtk.main_quit)
      bottom_buttons = gtk.HBox()
      cancel = gtk.Button("Cancel")
      cancel.connect("clicked", gtk.main_quit)
      
      prev = gtk.Button("Previous")
      prev.connect("clicked", self.do_prev)

      next = gtk.Button("Next")
      next.connect("clicked", self.do_next)
      
      bottom_buttons.pack_end(cancel, False, False)
      bottom_buttons.pack_end(next, False, False)
      bottom_buttons.pack_end(prev, False, False)
      bottom_buttons.show_all()
      self.vbox.pack_end(bottom_buttons, False, False)
      return gtk.main()

   def addScreen(self, screen, pos = 0) :
      #TODO: this shouldn't be done this way!
      # should setup a screens list to wich this screen would be appended
      self.vbox.add(screen.frame)

class AbsGtkScreen(AbsScreen) :

   def __init__(self, title="WARNING: I DON'T HAVE A TITLE!") :
      self.fields = {}
      self.widgets = []
      self.main_vbox = gtk.VBox()
      self.frame = gtk.Frame(title)
      self.frame.add(self.main_vbox)
      self.frame.show_all()

   def __registerField(self, name, widget) :
      self.fields[name] = widget

   def setTitle(self, title) :
      self.frame.set_title(title)

   def __addWidget(self, fieldName, w = None):
      if fieldName :
         self.__registerField(fieldName, w)
      self.widgets.append(w)
      #TODO: align to the left
      self.main_vbox.pack_start(w, False, False)

   def addLabel(self, fieldName, label, defaultValue = '', tooltip = "I DON'T HAVE A TOOLTIP", callBack = None) :
      w = gtk.Label(label)
      w.show()
      self.__addWidget(fieldName, w)

