#!/usr/bin/python
# -*- coding: iso-8859-1 -*-
import sys

if len(sys.argv) == 1 :
	print 'Syntax: %s <curses|qt|gtk>' %sys.argv[0]
	sys.exit(1)

mode = sys.argv[1]
if mode == 'curses' :
	from cwizard import *
	Wizard = AbsCursesWizard
	Screen = AbsCursesScreen
elif mode == 'qt' :
	from qtwizard import *
	Wizard = AbsQtWizard
	Screen = AbsQtScreen
elif mode == 'gtk' :
	from gtkwizard import *
	Wizard = AbsGtkWizard
	Screen = AbsGtkScreen
else :
	print 'Syntax: %s <curses|qt|gtk>' %sys.argv[0]
	sys.exit(1)

w = Wizard('AbsTk Minimalist Test')

scr1 = Screen('First Screen')
scr1.addLabel('Parameter1', '1234')
scr1.addLabel('Parameter2', 'abcd')
scr1.addLabel('Parameter3', 'xyz')

w.addScreen(scr1)

w.start()
