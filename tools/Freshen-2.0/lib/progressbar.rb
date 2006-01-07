# Copyright (C) 2004, Michael Homer
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
