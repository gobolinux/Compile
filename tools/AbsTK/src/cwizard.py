# -*- encoding iso-8859-1 -*-
#!/usr/bin/python

import curses, curses.textpad, traceback, string, time
from wizard import *

def setColor(color, fg, bg):
   try : curses.init_pair(color, fg, bg)
   except : pass

def debug(s):
   stdscr.addstr(0, 0, str(s))

class AbsCursesWizard(AbsWizard) :

   def __init__(self, name = '') :
      self.screens = []
      self.currentScreen = -1

   #detsch
   def clear(self, newName = '') :
      self.screens = []
      self.currentScreen = -1

   def start(self) :
      result = 0
      try :
         curses.noecho()
         curses.cbreak()
         stdscr.keypad(1)
         result = self.__main(stdscr)
         self.done()
      except :
         self.done()
         traceback.print_exc()
      return result

   def done(self):
      stdscr.keypad(0)
      curses.echo()
      curses.nocbreak()
      curses.endwin()

   def __drawScreen(self):
      stdscr.clear()
      stdscr.subwin(maxY-1,maxX,0,0).box()
      stdscr.addstr(maxY-3, 1, " Up/Dn: Move  Enter: Focus ", defaultColor)
      stdscr.addstr(maxY-3, maxX-35, " <- Back ", buttonColor)
      stdscr.addstr(maxY-3, maxX-25, " -> Next ", buttonColor)
      stdscr.addstr(maxY-3, maxX-15, " Esc: Cancel ", buttonColor)
      self.screens[self.currentScreen].draw()
      stdscr.refresh()

   def showMessageBox(self, message, options=['Ok','Cancel']) :
      area = stdscr.subwin(8, maxX-10, maxY/2-4, 5)
      area.clear()
      area.box()
      area.keypad(1)
      subarea = stdscr.subwin(5, maxX-15, maxY/2-3, 7)
      subarea.idlok(1)
      subarea.scrollok(1)
      subarea.addstr(0, 1, message, titleColor + curses.A_BOLD)
      k = 0
      sel = 0
      while k != 10 :
         l = 0
         for i in range(len(options)) :
            if i == sel :
               attr = widgetColor
            else :
               attr = buttonColor
            area.addstr(6, 2 + l, " "+options[i]+" ", attr)
            l = l + len(options[i]) + 4
         area.refresh()
         k = area.getch()
         if k == curses.KEY_RIGHT :
            sel = sel + 1
            if sel == len(options) :
               sel = 0
         elif k == curses.KEY_LEFT :
            sel = sel - 1
            if sel == - 1 :
               sel = len(options) - 1
      global uglyHack
      uglyHack = 1
      return options[sel]

   def __main(self, stdscr):
      global defaultColor, disabledColor, titleColor, buttonColor, widgetColor, widgetDisabledColor
      # Frame the interface area at fixed VT100 size
      if self.currentScreen == -1 :
         return
      k = 0
      self.__drawScreen()
      scrcount = len(self.screens)
      global uglyHack
      uglyHack = 0
      while 1:
         if self.currentScreen == scrcount - 1 :
            stdscr.addstr(maxY-3, maxX-25, " -> Done ", buttonColor)
         else :
            stdscr.addstr(maxY-3, maxX-25, " -> Next ", buttonColor)
         self.screens[self.currentScreen].draw()
         stdscr.move(maxY-1,maxX-1)
         if uglyHack == 1:
            k = 0
            uglyHack = 0
            stdscr.refresh()
         else :
            k = stdscr.getch()
         screen = self.screens[self.currentScreen]
         if not screen.processKey(k) :
            if k == curses.KEY_LEFT :
               if self.currentScreen > 0 :
                  self.currentScreen = self.currentScreen - 1
            elif k == curses.KEY_RIGHT :
               if not screen.nextCB or screen.nextCB():
                  if self.currentScreen < len(self.screens) - 1 :
                     self.currentScreen = self.currentScreen + 1
                  else :
                     stdscr.addstr(maxY-3, 1, " Enter: Confirm            ", defaultColor)
                     stdscr.addstr(maxY-3, maxX-25, " -> Done ", widgetColor)
                     stdscr.move(maxY-1,maxX-1)
                     k = stdscr.getch()
                     stdscr.addstr(maxY-3, 1, " Up/Dn: Move  Enter: Focus ", defaultColor)
                     if k == 10 :
                        return 1
            # Easter egg: MATRIX MODE :)
            elif k == ord('9') :
               setColor(1, curses.COLOR_GREEN, curses.COLOR_BLACK)
               setColor(2, curses.COLOR_RED, curses.COLOR_BLACK)
               setColor(3, curses.COLOR_GREEN, curses.COLOR_BLACK)
               setColor(4, curses.COLOR_BLACK, curses.COLOR_CYAN)
               setColor(5, curses.COLOR_BLUE, curses.COLOR_GREEN)
               defaultColor = curses.color_pair(1)
               disabledColor = curses.color_pair(2) + curses.A_BOLD
               titleColor = curses.color_pair(3)
               buttonColor = curses.color_pair(4)
               widgetColor = curses.color_pair(5)
               widgetDisabledColor = curses.color_pair(5) + curses.A_BOLD
               stdscr.clear()
               stdscr.addstr(0,0,"Knock, knock.")
               stdscr.getch()
               self.__drawScreen()
            elif k == ord('0') :
               setColor(1, curses.COLOR_WHITE, curses.COLOR_BLUE)
               setColor(2, curses.COLOR_BLACK, curses.COLOR_BLUE)
               setColor(3, curses.COLOR_YELLOW, curses.COLOR_BLUE)
               setColor(4, curses.COLOR_BLACK, curses.COLOR_WHITE)
               setColor(5, curses.COLOR_BLACK, curses.COLOR_CYAN)
               defaultColor = curses.color_pair(1)
               disabledColor = curses.color_pair(2) + curses.A_BOLD
               titleColor = curses.color_pair(3)
               buttonColor = curses.color_pair(4)
               widgetColor = curses.color_pair(5)
               widgetDisabledColor = curses.color_pair(5) + curses.A_BOLD
            elif k == 27 :
               return 0

   def addScreen(self, screen, pos = 0) :
      # Add the screen at the end
      if pos == 0 :
         self.screens.append(screen)
      # Add the screen right after the current one
      elif pos == -1 :
         self.screens.insert(self.currentScreen + 1, screen)
      # Add the screen at a specific possition
      else :
         self.screens.insert(pos,screen)
      if self.currentScreen == -1 :
         self.currentScreen = 0

   def removeScreen(self, screen) :
      self.screens.remove(screen)

class CursesWidget :

   def __init__() :
      self.height = 1
      self.tooltip = "I DON'T HAVE A TOOLTIP"
      self.enabled = 1

   def getHeight(self) :
      return self.height

   def isEnabled(self) :
      return self.enabled

   def getTooltip(self) :
      return self.tooltip

   def draw(self, drawable, x, y) :
      pass

   def makeAttr(self, inside, enabled, off, on, usein=None) :
      if not enabled :
         return off
      else :
         if inside and usein:
            return usein
         return on

class CursesLabel(CursesWidget) :
   def __init__(self, label) :
      self.height = 1
      #detsch:
      self.label = label

   #detsch:
   def setValue(self, newlabel) :
      self.label = newlabel

   #detsch:
   def getValue(self) :
      return self.label

   def draw(self, drawable, x, y) :
      attr = self.makeAttr(0, 1, disabledColor, defaultColor, defaultColor + curses.A_BOLD)
      drawable.addstr(y, x, self.label, attr)

class CursesList(CursesWidget) :

   def __init__(self, label, items, defaultValue, callBack, tooltip) :
      if len(items) < 5 and len(items) != 0:
         self.height = len(items)+1
         self.isCompact = 1
      else :
         self.height = 8
         self.isCompact = 0
      self.width = maxX - 10
      self.first = 0
      self.items = items
      self.value = 0
      self.scrollH = 0
      for i in range(len(items)) :
         if items[i] == defaultValue :
            self.value = i
      self.inside = 0
      self.enabled = 1
      self.label = label
      self.callBack = callBack
      self.tooltip = tooltip
      if self.value >= self.first + self.height:
         self.first = self.value

   def setEnabled(self, enabled) :
      self.enabled = enabled
      if not enabled :
         self.inside = 0

   def setValue(self, valueTuple) :
      self.items = valueTuple[0][:]
      try :
         self.value = self.items.find(valueTuple[1])
      except :
         self.value = 0


   #detsch ([:] to copy the list instead of passing a reference)
   def getValue(self) :
      if len(self.items) >  self.value:
         v = self.items[self.value]
      else :
         v = 0
      return (self.items[:], v)

   def draw(self, drawable, x, y) :
      attrdefault = self.makeAttr(self.inside, self.enabled, disabledColor, defaultColor, defaultColor + curses.A_BOLD)
      drawable.addstr(y, x, self.label, attrdefault)
      if self.isCompact :
         pos = y+1
         for (item, i) in zip(self.items, range(len(self.items))) :
            if self.value == i:
               drawable.addstr(pos, x, "(*) ", attrdefault)
            else :
               drawable.addstr(pos, x, "( ) ", attrdefault)
            drawable.addstr(pos, x+4, item, attrdefault)
            pos = pos + 1
      else :
         attrlist = self.makeAttr(self.inside, self.enabled, widgetDisabledColor, widgetColor)
         attritem = self.makeAttr(self.inside, self.enabled, widgetDisabledColor, widgetColor + curses.A_STANDOUT, widgetColor + curses.A_STANDOUT + curses.A_BOLD)
         w = drawable.subwin(self.height-1, self.width, y+1, x)
         w.attrset(attrdefault)
         w.border()
         i = self.first
         drawable.addstr(y+(((self.value+0.0)/max(len(self.items),1))*(self.height-3))+2, x+self.width-1, "*", attrdefault)
         ypos = y+2
         for item in self.items[i:i+5] :
            cropItem = item[self.scrollH:self.width-2+self.scrollH].ljust(self.width-2)
            if self.value == i:
               drawable.addstr(ypos, x+1, cropItem, attritem)
            else :
               drawable.addstr(ypos, x+1, cropItem, attrlist)
            i = i + 1
            ypos = ypos + 1

   def processKey(self, key) :
      if self.inside :
         v = self.value
         if key == 10 :
            self.inside = 0
            if self.callBack != None :
               self.callBack()
         elif key == curses.KEY_UP :
            v = v - 1
            if v == -1 :
               v = 0
            self.value = v
         elif key == curses.KEY_DOWN :
            v = v + 1
            if v == len(self.items) :
               v = len(self.items) - 1
            self.value = v
         elif key == curses.KEY_RIGHT:
            self.scrollH = self.scrollH + 10
            return -2
         elif key == curses.KEY_LEFT:
            if self.scrollH > 0:
               self.scrollH = self.scrollH - 10
            return -2
         if v < self.first :
            self.first = v
         if v >= self.first+(self.height-3) :
            self.first = v-((self.height-3)-1)
      else :
         if key == 10 and self.enabled :
            self.inside = 1
         elif key == curses.KEY_UP :
            return -1
         elif key == curses.KEY_DOWN or key == 9 :
            return 1
      return 0

class CursesDropList(CursesWidget) :

   def __init__(self, label, items, defaultValue, callBack, tooltip) :
      self.height = 1
      self.width = maxX - 10
      self.first = 0
      self.items = items
      self.value = 0
      self.maxitemlength = len(items[0])
      for i in range(len(items)) :
         if items[i] == defaultValue :
            self.value = i
	    self.maxitemlength = max(self.maxitemlength,len(self.items[i]))
      self.inside = 0
      self.enabled = 1
      self.label = label
      self.callBack = callBack
      self.tooltip = tooltip
      self.scrollH = 0

   def setEnabled(self, enabled) :
      self.enabled = enabled
      if not enabled :
         self.inside = 0

   def setValue(self, value) :
      v = self.value
      try :
         self.value = self.items.find(value)
      except :
         self.value = v

   def getValue(self) :
      if len(self.items) > self.value:
         return self.items[self.value]
      else :
         return self.items[0]

   def draw(self, drawable, x, y) :
      attr = self.makeAttr(self.inside, self.enabled, disabledColor, defaultColor, defaultColor + curses.A_BOLD)
      drawable.addstr(y, x, '%s :'%self.label, attr)
      pos = x+len(self.label)+3
      drawable.addstr(y, pos, self.items[self.value], attr)
      self.x = x
      self.y = y

   def processKey(self, key) :
      if key == 10 and self.enabled :
         areaX = min(maxX-6-self.maxitemlength,self.x+len(self.label)+3)
         areaY = max(6,self.y-(min(8,len(self.items)+2)/2)+1)
         areaHeight = min(8,len(self.items))
         areaWidth = min(maxX-14,self.maxitemlength)
         area = stdscr.subwin(areaHeight+2, areaWidth+4, areaY, areaX)
         area.clear()
         area.box()
         area.keypad(1)
         k = 0
         sel = self.value
         while True :
            offset = 0
            if len(self.items) > areaHeight :
               offset = sel - (areaHeight / 2)
            for i in range(min(len(self.items),areaHeight)) :
               if i+offset == sel :
                  attr = widgetColor
               else :
                  attr = buttonColor
               area.addstr(i+1, 1, self.items[offset+i], attr)
	       for j in range(self.maxitemlength-len(self.items[offset+i])) :
                  area.addstr(i+1,len(self.items[offset+i])+j+1,' ', attr)
            area.refresh()
            k = area.getch()
            if k == curses.KEY_UP :
               sel = sel - 1
               if sel == - 1 :
                  sel = len(self.items) - 1
            elif k == curses.KEY_DOWN :
               sel = sel + 1
               if sel == len(self.items):
                  sel = 0
            elif k == 10 :
               self.value = sel
               if self.callBack != None :
                  self.callBack()
               break
         global uglyHack
         uglyHack = 1
         return 0 
      elif key == curses.KEY_UP :
         return -1
      elif key == curses.KEY_DOWN or key == 9 :
         return 1
      return 0

class CursesTextBox(CursesWidget) :

   def __init__(self, label, defaultValue, callBack, tooltip) :
      self.height = 18
      self.width = maxX - 10
      self.first = 0
      self.label = label
      self.value = defaultValue.split("\n")
      self.inside = 0
      self.enabled = 1
      self.callBack = callBack
      self.tooltip = tooltip
      self.scrollH = 0

   def setEnabled(self, enabled) :
      self.enabled = enabled
      if not enabled :
         self.inside = 0

   def setValue(self, value) :
      self.value = value.split("\n")

   def getValue(self):
      return string.join(self.value, "\n")

   def draw(self, drawable, x, y) :
      attr = self.makeAttr(self.inside, self.enabled, disabledColor, defaultColor, defaultColor + curses.A_BOLD)
      drawable.addstr(y, x, self.label, attr)
      drawable.subwin(self.height-1, self.width, y+1, x).border()
      i = self.first
      pc = (self.first+0.0)/len(self.value)
      pch = pc * (self.height-3)
      drawable.addstr(y+int(pch)+2, x+self.width-1, "*")
      ypos = y+2
      for item in self.value[i:i+self.height-3] :
         cropItem = item[self.scrollH:self.width-2+self.scrollH].ljust(self.width-2)
         drawable.addstr(ypos, x+1, cropItem, attr)
         i = i + 1
         ypos = ypos + 1

   def processKey(self, key) :
      if self.inside :
         f = self.first
         if key == 10 :
            self.inside = 0
            if self.callBack != None :
               self.callBack()
         elif key == curses.KEY_UP :
            f = f - 1
            if f == -1 :
               f = 0
         elif key == curses.KEY_DOWN :
            f = f + 1
            if f == len(self.value) :
               f = len(self.value) - 1
         elif key == curses.KEY_RIGHT:
            self.scrollH = self.scrollH + 10
            return -2
         elif key == curses.KEY_LEFT:
            if self.scrollH > 0:
               self.scrollH = self.scrollH - 10
            return -2
         self.first = f
      else :
         if key == 10 and self.enabled :
            self.inside = 1
         elif key == curses.KEY_UP :
            return -1
         elif key == curses.KEY_DOWN or key == 9 :
            return 1
      return 0


class CursesCheckList(CursesWidget) :

   def __init__(self, label, items, defaultValue, callBack, tooltip) :
      if len(items) < 5 :
         self.height = len(items)+1
         self.isCompact = 1
      else :
         self.height = 8
         self.isCompact = 0
      #detsch
      self.width = maxX - 10
      self.first = 0
      self.items = items
      self.value = defaultValue
      self.current = 0
      self.inside = 0
      self.enabled = 1
      self.label = label
      self.callBack = callBack
      self.tooltip = tooltip
      self.scrollH = 0

   def setEnabled(self, enabled) :
      self.enabled = enabled
      if not enabled :
         self.inside = 0

   #detsch
   def setValue(self, valueTuple) :
      self.items = valueTuple[0][:]
      self.value = map(lambda item : item in valueTuple[1], self.items)
      

   def getValue(self):
      result = []
      for (value, item) in zip(self.value, self.items) :
         if value == 1 :
            result.append(item)
      #detsch ([:] para copiar a lista, e nao passar a referencia)
      return (self.items[:], result)

   def draw(self, drawable, x, y) :
      attrdefault = self.makeAttr(self.inside, self.enabled, disabledColor, defaultColor, defaultColor + curses.A_BOLD)
      drawable.addstr(y, x, self.label, attrdefault)
      attrlist = self.makeAttr(self.inside, self.enabled, widgetDisabledColor, widgetColor)
      attritem = self.makeAttr(self.inside, self.enabled, widgetDisabledColor, widgetColor + curses.A_STANDOUT, widgetColor + curses.A_STANDOUT + curses.A_BOLD)

      curr = self.current
      i = self.first
      if self.isCompact :
         ypos = y+1
         xpos = x
      else :
         w = drawable.subwin(self.height-1, self.width, y+1, x)
         w.attrset(attrdefault)
         w.border()
         drawable.addstr(y+(((curr+0.0)/len(self.items))*(self.height-3))+2, x+self.width-1, "*", attrdefault)
         ypos = y+2
         xpos = x+1
      currItem = self.items[curr]
      for (item, value) in zip(self.items[i:i+(self.height-3)], self.value[i:i+(self.height-3)]) :
         cropItem = item[self.scrollH:self.width-6+self.scrollH].ljust(self.width-6)
         if item == currItem :
            at = attritem
         else :
            at = attrlist
         if value == 1:
            drawable.addstr(ypos, xpos, "[x] " + cropItem, at)
         else :
            drawable.addstr(ypos, xpos, "[ ] " + cropItem, at)
         ypos = ypos + 1

   def processKey(self, key) :
      if self.inside :
         c = self.current
         if key == 10 :
            self.inside = 0
            if self.callBack != None :
               self.callBack()
         if key == 32 :
            if self.value[c] == 0 :
               self.value[c] = 1
            else :
               self.value[c] = 0
            #detsch
            if self.callBack != None :
               self.callBack()

         elif key == curses.KEY_UP :
            c = c - 1
            if c == -1 :
               c = 0
            self.current = c
         elif key == curses.KEY_DOWN :
            c = c + 1
            if c == len(self.items) :
               c = len(self.items) - 1
            self.current = c
         elif key == curses.KEY_RIGHT:
            self.scrollH = self.scrollH + 10
            return -2
         elif key == curses.KEY_LEFT:
            if self.scrollH > 0:
               self.scrollH = self.scrollH - 10
            return -2
         if not self.isCompact :
            if c < self.first :
               self.first = c
            if c >= self.first+(self.height-3) :
               self.first = c-((self.height-3)-1)
      else :
         if key == 10 and self.enabled :
            self.inside = 1
         elif key == curses.KEY_UP :
            return -1
         elif key == curses.KEY_DOWN or key == 9 :
            return 1
      return 0

class CursesBoolean(CursesWidget) :

   def __init__(self, label, value, callBack, tooltip) :
      self.callBack = callBack
      self.height = 1
      self.label = label
      self.value = value
      self.enabled = 1
      self.tooltip = tooltip

   def setEnabled(self, enabled) :
      self.enabled = enabled

   def setValue(self, value) :
      self.value = value

   def getValue(self):
      return self.value

   def draw(self, drawable, x, y) :
      attr = self.makeAttr(0, self.enabled, disabledColor, defaultColor, defaultColor + curses.A_BOLD)
      if self.value == 1 :
         drawable.addstr(y, x, "[x] " + self.label, attr)
      else :
         drawable.addstr(y, x, "[ ] " + self.label, attr)

   def processKey(self, key) :
      if key == 10 and self.enabled :
         if self.value == 0 :
            self.value = 1
         else :
            self.value = 0
         if self.callBack != None :
            self.callBack()
      elif key == curses.KEY_UP :
         return -1
      elif key == curses.KEY_DOWN or key == 9 :
         return 1
      return 0

class CursesButton(CursesWidget) :

   def __init__(self, label, value, callBack, tooltip) :
      self.height = 3
      self.label = label
      self.value = value
      self.enabled = 1
      self.callBack = callBack
      self.tooltip = tooltip

   def setEnabled(self, enabled) :
      self.enabled = enabled

   def setValue(self, value) :
      self.value = value

   def getValue(self):
      return self.value

   def draw(self, drawable, x, y) :
      attr = self.makeAttr(0, self.enabled, buttonColor, buttonColor)
      area = drawable.subwin(3, len(self.label)+4, y, x)
      area.bkgd(" ",buttonColor)
      area.border()
      drawable.addstr(y+1, x+2, self.label, attr)

   def processKey(self, key) :
      if key == 10 and self.enabled :
         if self.callBack != None :
            self.callBack()
      elif key == curses.KEY_UP :
         return -1
      elif key == curses.KEY_DOWN or key == 9 :
         return 1
      return 0

class CursesEntry(CursesWidget) :

   def __init__(self, label, value, callBack, tooltip) :
      self.height = 1
      self.width = maxX - 10 - len(label)
      self.label = label
      self.value = value
      self.inside = 0
      self.cursor = len(value)
      self.enabled = 1
      self.callBack = callBack
      self.tooltip = tooltip

   def setEnabled(self, enabled) :
      self.enabled = enabled
      if not enabled :
         self.inside = 0

   def setValue(self, value) :
      self.value = value
      self.cursor = len(value)

   def getValue(self):
      return self.value

   def draw(self, drawable, x, y) :
      attr = self.makeAttr(self.inside, self.enabled, disabledColor, defaultColor, defaultColor + curses.A_BOLD)
      field = self.value[:self.width].ljust(self.width)
      cur = self.cursor
      label = self.label + " ["
      drawable.addstr(y, x, label, attr)
      drawable.addstr(y, x+len(label)+len(field), "]", attr)
      attr = self.makeAttr(self.inside, self.enabled, widgetDisabledColor, widgetColor, widgetColor + curses.A_STANDOUT + curses.A_BOLD)
      if self.inside:
         left = field[:cur]
         mid = field[cur:cur+1]
         right = field[cur+1:]
         drawable.addstr(y, x+len(label), left, attr)
         drawable.addstr(y, x+len(label)+len(left), mid, attr + curses.A_STANDOUT)
         drawable.addstr(y, x+len(label)+len(left)+1, right, attr)
      else:
         drawable.addstr(y, x+len(label), field, attr)

   def processKey(self, key) :
      if self.inside :
         if key == 10 :
            self.inside = 0
            if self.callBack != None :
               self.callBack()
         elif (key >= 32 and key <= 126) or (key >= 128 and key <= 255) :
            cur = self.cursor
            self.value = self.value[:cur] + chr(key) + self.value[cur:]
            self.cursor = cur + 1
         elif key == 127 or key == 263:
            cur = self.cursor
            if cur > 0 :
               self.value = self.value[:cur-1] + self.value[cur:]
               self.cursor = cur - 1
         elif key == 330 :
            cur = self.cursor
            self.value = self.value[:cur] + self.value[cur+1:]
         elif key == curses.KEY_LEFT :
            cur = self.cursor
            if cur > 0 :
               self.cursor = cur - 1
         elif key == curses.KEY_RIGHT :
            cur = self.cursor
            if cur < len(self.value) :
               self.cursor = cur + 1
         return 2
      else :
         if key == 10 and self.enabled :
            self.inside = 1
         elif key == curses.KEY_UP :
            return -1
         elif key == curses.KEY_DOWN or key == 9 :
            return 1
      return 0

class CursesPassword(CursesEntry) :

   def draw(self, drawable, x, y) :
      saveField = self.value
      pw = ""
      for c in saveField :
         pw = pw + "*"
      self.value = pw
      CursesEntry.draw(self, drawable, x, y)
      self.value = saveField

class AbsCursesScreen(AbsScreen) :

   def __init__(self, title="WARNING: I DON'T HAVE A TITLE!") :
      self.fields = {}
      self.widgets = []
      self.focusWidgets = []
      self.pad = curses.newpad(maxX-2 + 100, maxY-4 + 100)
      self.pad.bkgd(" ",curses.color_pair(1))
      self.focus = -1
      self.vscroll = 0
      self.title = title
      self.nextCB = None

   def draw(self) :
      self.pad.clear()
      self.pad.addstr(0, 0, self.title, curses.color_pair(3) + curses.A_BOLD)
      at = 2
      focusAt = 2
      focusHeight = 0
      if self.focus > -1 :
         focused = self.focusWidgets[self.focus]
      else :
         focused = None
      vscroll = self.vscroll
      for widget in self.widgets :
         if widget == focused :
            self.pad.addstr(at, 1, ">", titleColor + curses.A_BOLD)
            focusAt = at
            focusHeight = widget.getHeight()
         widget.draw(self.pad, 3, at)
         at = at + widget.getHeight() + 1
      padheight = maxY - 5
      if (focusAt + focusHeight) >= (vscroll + padheight) :
         vscroll = vscroll + ((focusAt+focusHeight) - (vscroll+padheight))
         self.vscroll = vscroll
      if focusAt < vscroll :
         vscroll = focusAt
         if vscroll == 2 :
            vscroll = 0
         self.vscroll = vscroll
      drawBar = 0
      drawBarUp = disabledColor
      drawBarDown = disabledColor
      if vscroll > 2 :
         drawBar = 1
         drawBarUp = buttonColor
      if at - 1 > vscroll + padheight :
         drawBar = 1
         drawBarDown = buttonColor
      if drawBar :
         x = maxX - 3
         barHeight = (vscroll + padheight - 1) - (vscroll+1)
         for y in range(vscroll+1, vscroll + padheight - 1) :
            self.pad.addstr(y, x, " ", widgetColor)
         self.pad.addstr( vscroll + 1 + int(((focusAt+0.0)/(at-1)) * (barHeight-1)), maxX - 3, " ", buttonColor )
         self.pad.addstr(vscroll, x, "^", drawBarUp)
         self.pad.addstr(vscroll + padheight - 1 , x, "v", drawBarDown)

      self.pad.refresh(vscroll, 0, 1, 1, padheight, maxX-2)
      if focused :
         stdscr.addstr(maxY - 1, 0, focused.getTooltip().ljust(maxX)[:maxX-1], widgetColor)

   def processKey(self, key) :
      if self.focus == -1 :
         return
      cmd = self.focusWidgets[self.focus].processKey(key)
      if cmd == -1 :
         self.focus = self.focus - 1
         if self.focus == -1 :
            self.focus = len(self.focusWidgets) - 1
      elif cmd == 1 :
         self.focus = self.focus + 1
         if self.focus == len(self.focusWidgets) :
            self.focus = 0
      elif cmd == 0 :
         return 0
      return 1

   def __registerField(self, name, widget) :
      self.fields[name] = widget

   def __registerFocus(self, widget) :
      self.focusWidgets.append(widget)
      if self.focus == -1 :
         self.focus = 0

   def setTitle(self, title) :
      self.title = title

   def __addWidget(self, fieldName, w = None, focus = 1):
      if fieldName :
         self.__registerField(fieldName, w)
      if focus :
         self.__registerFocus(w)
      self.widgets.append(w)

   def addLabel(self, fieldName, label, defaultValue = '', tooltip = "I DON'T HAVE A TOOLTIP", callBack = None) :
      w = CursesLabel(label)
      self.__addWidget(fieldName, w, 0)

   #detsch
   def addList(self, fieldName, label, defaultValueTuple = ([],''), tooltip = "I DON'T HAVE A TOOLTIP", callBack = None) :
      w = CursesList(label, defaultValueTuple[0], defaultValueTuple[1], callBack, tooltip)
      self.__addWidget(fieldName, w)

   def addDropList(self, fieldName, label='', defaultValueTuple=([],''), tooltip='', callBack=None) :
      w = CursesDropList(label, defaultValueTuple[0], defaultValueTuple[1], callBack, tooltip)
      self.__addWidget(fieldName, w)

   def addBoolean(self, fieldName, label, defaultValue = 0, tooltip = "I DON'T HAVE A TOOLTIP", callBack = None) :
      w = CursesBoolean(label, defaultValue, callBack, tooltip)
      self.__addWidget(fieldName, w)

   def addLineEdit(self, fieldName, label, defaultValue = '', tooltip = "I DON'T HAVE A TOOLTIP", callBack = None) :
      w = CursesEntry(label, defaultValue, callBack, tooltip)
      self.__addWidget(fieldName, w)

   def addPassword(self, fieldName, label, defaultValue = '', tooltip = "I DON'T HAVE A TOOLTIP", callBack = None) :
      w = CursesPassword(label, defaultValue, callBack, tooltip)
      self.__addWidget(fieldName, w)

   def addMultiLineEdit(self, fieldName, label, defaultValue = '', tooltip = "I DON'T HAVE A TOOLTIP", callBack = None) :
      w = CursesTextBox(label, defaultValue, callBack, tooltip)
      self.__addWidget(fieldName, w)

   #detsch
   def addCheckList(self, fieldName, label, defaultValueTuple = ([],[]), tooltip = "I DON'T HAVE A TOOLTIP", callBack = None) :
      defaultChecks = map(lambda item : item in defaultValueTuple[1], defaultValueTuple[0])
      w = CursesCheckList(label, defaultValueTuple[0], defaultChecks, callBack, tooltip)
      self.__addWidget(fieldName, w)

   def addButton(self, fieldName, label, defaultValue = '', tooltip = "I DON'T HAVE A TOOLTIP", callBack = None) :
      w = CursesButton(label, defaultValue, callBack, tooltip)
      self.__addWidget(fieldName, w)
   
   def addImage(self, fileName) :
      pass
   
   def onValidate(self, nextCB) :
      self.nextCB = nextCB

global stdscr
global defaultColor, disabledColor, titleColor, buttonColor, widgetColor
stdscr=curses.initscr()
global maxX, maxY
(maxY, maxX) = stdscr.getmaxyx()
curses.start_color()
setColor(1, curses.COLOR_WHITE, curses.COLOR_BLUE)
setColor(2, curses.COLOR_BLACK, curses.COLOR_BLUE)
setColor(3, curses.COLOR_YELLOW, curses.COLOR_BLUE)
setColor(4, curses.COLOR_BLACK, curses.COLOR_WHITE)
setColor(5, curses.COLOR_BLACK, curses.COLOR_CYAN)
defaultColor = curses.color_pair(1)
disabledColor = curses.color_pair(2) + curses.A_BOLD
titleColor = curses.color_pair(3)
buttonColor = curses.color_pair(4)
widgetColor = curses.color_pair(5)
widgetDisabledColor = curses.color_pair(5) + curses.A_BOLD

stdscr.bkgd(" ",curses.color_pair(1))
