# Module for handling screen functions
# Presently, only terminal dimensions
# Can be module or mixin
# Mixin defines @@width and @@height
# This is to avoid extra stty calls
# Copyright 2004 Michael Homer
module Screen
	Colours = {}
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
	# Mwaha efficiency.  Sort of makes a mockery of that whole "Constant" thing, though
	# I suppose being a constant isn't the same as Colours.freeze
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