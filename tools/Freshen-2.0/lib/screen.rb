# Copyright (C) 2004-2007 Michael Homer
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Module for handling terminal functions
# Presently, only terminal dimensions and colours.
# Can be module or mixin
# Mixin defines @@width and @@height
# This is to avoid extra stty calls
module Screen
	Colours = Hash.new("")
	@@width = @@height = false
	def Screen.width
		`stty size` =~/(\d+) (\d+)/
		$2.to_i
	end
	def Screen.height
		`stty size` =~/(\d+) (\d+)/
		$1.to_i
	end
	def width
		if !@@width
			@@width=Screen.width
		end
		@@width
	end
	def height
		if !@@height
			@@height=Screen.height
		end
		@@height
	end
	def Screen.colour
		if STDOUT.tty? and Colours.length==0
			Colours['blueboldwhite']    = "\033[1;44;37m"
			Colours['red']    = "\033[1;31m"
			Colours['blue']   = "\033[1;34m"
			Colours['green']  = "\033[1;32m"
			Colours['yellow'] = "\033[1;33m"
			Colours['cyan']   = "\033[36m"
			Colours['brown']  = "\033[33m"
			Colours['normal'] = "\033[0m"
		end
		return Colours
	end
end