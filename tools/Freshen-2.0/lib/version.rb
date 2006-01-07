# Copyright (C) 2004, Michael Homer
# Version: represents a version
class Version
	include Comparable # Get free comparisons
	def initialize(version)
		@version = version.to_s
	end
	def to_s
		if @version
			@version
		else
			"none"
		end
	end
	def set?
		@version!="0"
	end
	def version= (x)
		@version=x.to_s
	end
	def version
		"#{@version}"
	end
	def <=> (x)
		if @version.nil? and x.nil
			return 0
		elsif @version.nil?
			return -1
		elsif x.nil?
			return 1
		end
		if !x.is_a?(Version)
			raise "#<#{x.class}:#{x.id}> (#{x.inspect}) is not a Version"
		end
		# assume x.is_a(Version), there shouldn't be a need to compare anything else
		# This is a beautiful piece of code.
		# It splits on . or -, and on boundaries between alphabetic and numerics, and 
		#  removes empties.
		mytokens = @version.split(/ [-.] | (\d)(?=[a-z]) /x).delete_if {|y| y==""}
		xstokens = x.version.split(/ [-.] | (\d)(?=[a-z]) /x).delete_if {|y| y==""}
		returnvalue = 0
		mytokens.length.times do |i|
			xv = xstokens[i]
			mv = mytokens[i]
			if mv.to_i.to_s == mv && xv.to_i.to_s == xv
				mv = mv.to_i
				xv = xv.to_i
			end
			if xv.nil?
				returnvalue = 1
			elsif mv > xv
				returnvalue = 1
				break
			elsif mv < xv
				returnvalue = -1
				break			
			end
		end
		returnvalue
	end
end