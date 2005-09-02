#!/usr/bin/ruby (require)

##################################################
# Changelog
##################################################

# 09/02/2004 - [calica] Initial version from Michael Homer's Freshen

# This class reads and parses shell script config file
# copied from Michael Homer's Freshen

require 'shellwords'

class GoboParseConfig < Hash
  public        # Public attributes
  private       # Private instance/class variables
  public        # Public methods
    def initialize(filename, inheritfrom=nil)
        super(inheritfrom)
# 		if inheritfrom
# 			self = inheritfrom.vars
# 		else
# 			self = Hash.new
# 		end
		inarray = false
		thisarray = "" #look, I know these are dirty blah blah
		file = File.open(filename)
		file.readlines.each {
			|line|
			line.strip!
			if !line.empty? and line.slice(0,1) != "#"
				words = Shellwords.shellwords(line)
				words[0].gsub!(/\$([a-zA-Z]+)/) {self[$1]}
				spl = words[0].split("=", 2)
				if spl[1] == "("
					inarray = true
					self.store spl[0], Array.new
					thisarray = spl[0]
				elsif inarray
					if words[0] == ")"
						inarray = false
					else
						self[thisarray].push words[0]
					end
				else
					self.store(spl[0], spl[1])
				end
			end
		}
    end  # End of initialize()
# 	def [] (x)
# 		@vars[x]
# 	end  # End of [](x)
# 	def member? (x)
# 		@vars.member?(x)
# 	end  # End of member?
  protected     # Protected methods
  private       # Private methods
end

