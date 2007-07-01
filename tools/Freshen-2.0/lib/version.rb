# Copyright (C) 2004, Michael Homer
# Version: represents a version
class Version
	include Comparable # Get free comparisons
	def initialize(version)
		@version = version.to_s
	end
	def to_s
		if set?
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
			raise "#<#{x.class}:#{x.object_id}> (#{x.inspect}) is not a Version"
		end
		# assume x.is_a(Version), there shouldn't be a need to compare anything else
		# It splits on . or -, and on boundaries between alphabetic and numerics, and 
		#  then removes empties.
		mytokens = @version.split(/ [-.] | (\d)(?=[a-z]) /x).delete_if {|y| y==""}
		xstokens = x.version.split(/ [-.] | (\d)(?=[a-z]) /x).delete_if {|y| y==""}
		mytokens = @version.split(/ [-.] | (\d+)(?=[a-z]) | ([a-z]+)(?=\d)/x).delete_if {|y| y==""}
		xstokens = x.version.split(/ [-.] | (\d+)(?=[a-z]) | ([a-z]+)(?=\d)/x).delete_if {|y| y==""}
		returnvalue = 0
		mytokens.length.times do |i|
			xv = xstokens[i]
			mv = mytokens[i]
			#puts "comparing #{xv} and #{mv}"
			if mv.to_i.to_s == mv && xv.to_i.to_s == xv
				mv = mv.to_i
				xv = xv.to_i
			end
			if mv.to_s == 'pre' && xv.to_s != 'pre'
				returnvalue = -1
				break
			elsif xv.to_s == 'pre' && mv.to_s != 'pre'
				returnvalue = 1
				break
			elsif mv.to_s == "rc" && mv.to_s != xv.to_s
				returnvalue = -1
				break
			elsif mv.to_s == "r"
				if mytokens[i+1] == '1' && xv.to_s == '0'
					# Ignore
				elsif mytokens[i+1] == '1' && xv.nil?
					returnvalue = 0
					break
				elsif xv.to_s != "r" && xv.to_s.to_i.to_s == xv.to_s
					returnvalue = -1
					break
				elsif mytokens[i+1].nil?
					returnvalue = 1
					break
				elsif xstokens[i+1].nil?
					returnvalue = -1
					break
				end
				# Otherwise, ignore
			elsif xv.to_s == "r" && !xstokens[i+1].nil?
				returnvalue = 1
				break
			elsif xv.nil? && (mv=='0' || mv == nil || (mv=='r' && mytokens[i+1]=='1'))
				returnvalue = 0
			elsif xv.nil?
				return 1
			elsif mv.to_i>0 && xv.to_s.match(/^[a-z]+$/)
				returnvalue = 1
				break
			elsif xv.to_i>0 && mv.to_s.match(/^[a-z]+$/)
				returnvalue = -1
				break
			elsif mv > xv
				returnvalue = 1
				break
			elsif mv < xv
				returnvalue = -1
				break
			end
		end
		#puts "got value #{returnvalue}"
		if returnvalue == 0 and xstokens.length>mytokens.length
			mytokens.length.upto(xstokens.length) {|i|
				returnvalue = -1 unless xstokens[i].to_s=='0' || xstokens[i].nil?
				if xstokens[i].to_s=='r' && xstokens[i+1].to_s=='1'
					returnvalue = 0
					break
				end
			}
		end
		#returnvalue = -1 if returnvalue == 0 and xstokens.length>mytokens.length
		returnvalue
	end
end