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

# Handles drawing a generic progress bar
# ProgressBar.new (maximum[, initial value])
# progressbar.inc [(amount to increment)] returns false on failure
# progressbar.draw draws the progress bar
class ProgressBar
	def initialize (max, val=0)
		@max = max
		@val = val
		@width = Screen.width
		@height = Screen.height
		@barwidth = @width-7
		@dotcount = 0
	end
	def draw()
		if STDOUT.tty? and @dotcount!=(@dotcount=(@barwidth*@val/@max).floor)
			dots = "#" * @dotcount + " " * (@barwidth-@dotcount)
			pbo = "[#{dots}] #{(@val.to_f/@max.to_f*100).floor.to_s.rjust(3)}%"
			print pbo + "\015" # + "\010" * pbo.length
			STDOUT.flush
		end
	end
	def inc (amt=1)
		@val = @val + amt
		if @val>@max
			@val = @max
			return false
		end
	end
end
