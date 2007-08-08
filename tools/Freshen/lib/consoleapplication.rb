# Copyright (C) 2007 Michael Homer
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

require 'screen'

Param = Struct.new(:canonicalName, :description, :hasArgs, :short, :modeOpt)

class ConsoleApplication
	@@toExec = []
	
	attr_accessor :credits, :description, :usage, :version, :name
	attr_reader :list, :mode
	
	def initialize
		trap(0) {
			print "\033];\007"
		}
		@params = {}
		@paramData = {}
		@name = self.class.to_s
		@modes = []
		@mode = nil
		@list = []
		addMode(:version, 'Displays version information', :v)
		addMode(:help, 'Displays this help')
		@@toExec.each {|l|
			l.call(self)
		}
	end
	
	def start
		begin
		current = nil
		ARGV.each {|p|
			o = nil
			o = p[1,p.length].to_sym if p.length == 2
			o = p[2,p.length].to_sym if p.length > 2
			if @list.length>0 || (p[0] != ?- && !current)
				@list.push p
			elsif o && @params.has_key?(o)
				o = @params[o].canonicalName
				current = @params[o].hasArgs ? o : nil
				
				@paramData[o] = {:list=>[], :occurrences=>1}
				if @params[o].modeOpt
					raise "Only one mode option may be specified. Mode is #{@mode}, #{p} was also specified." if @mode
					@mode = o
				end
			elsif !current
				raise "No such option: #{p}"
			elsif p == '--'
				current = nil
			else
				if current
					@paramData[current][:list].push p
				else
					@list.push p
				end
			end
		}
		
		# The automatic mapping of ARGV to STDIN isn't terribly useful. Disable it unless it's wanted.
		ARGV.clear

		@mode = :default if !@mode
		
		case @mode
		when :help
			displayHelp
		when :version
			displayVersion
		else
			run if respond_to? :run
			begin
				send("run_#{@mode.to_s}".to_sym, @list) if respond_to? "run_#{@mode.to_s}".to_sym
			rescue ArgumentError
				send("run_#{@mode.to_s}".to_sym)
			end
		end
		rescue SystemExit
			# Ignore
		rescue Exception
			error $!
			backtrace
		end
	end
	
	def option(name)
		@paramData[name]
	end
	
	def displayHelp
		puts "#{@name} #{@version}"
		puts @description
		puts
		puts "Usage: #{@name}: #{@usage}"
		puts
		puts "Operational modes:"
		@modes.sort.each {|key|
			next if @params[key].description.nil?
			puts "--#{key}#{@params[key].short ? ', -' + @params[key].short.to_s : ''}"
			puts "        #{@params[key].description}"			
		}
		puts "Options:"
		@params.keys.sort.each {|key|
			next if !@params[key].description or @params[key].modeOpt or @params[key].description.nil?
			puts "--#{key}#{@params[key].short ? ', -'+@params[key].short.to_s : ''}"
			puts "        #{@params[key].description}"
		}
	end
	
	def displayVersion
		puts "#{@name} #{@version}"
		puts @credits
		puts
    puts Screen.wordwrap("This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
", 1)
	end
		
	def addMode(long, helpText, short=nil)
		@params[long] = Param.new(long, helpText, false, short, true)
		@modes.push long
		addOptAlias(short, long) if short
	end
	
	def addOptBoolean(long, helpText, short=nil)
		@params[long] = Param.new(long, helpText, false, short, false)
		addOptAlias(short, long) if short
	end
	
	def addOptAlias(from, to)
		to = @params[to].canonicalName while @params[to].canonicalName != to
		@params[from] = Param.new(to)
	end
	
	def addOpt(long, helpText, short=nil)
		@params[long] = Param.new(long, helpText, true, short, false)
		addOptAlias(short, long) if short
	end
	
	def title(str)
		puts "\033]2;#{@name}: #{str}\007#{Screen.colour['cyan']}#{@name}#{Screen.colour['normal']}: #{str}"
	end
	
	def out(str)
		puts "#{Screen.colour['cyan']}#{@name}#{Screen.colour['normal']}: #{str}"
	end
	
	def error(msg)
		puts "#{Screen.colour['cyan']}#{@name}#{Screen.colour['normal']}: #{Screen.colour['red']}Error:#{Screen.colour['normal']} #{msg}"
	end
	
	def debug(msg)
		puts "#{@name}: #{msg}" if debug?
	end
	
	def debug?
		true
	end
	
	def self.has_mode(sym, description=nil, short=nil)
		@@toExec.push lambda {|s|
			s.addMode(sym, description, short)
		}
	end

	def self.has_opt(sym, description=nil, short=nil)
		@@toExec.push lambda {|s|
			s.addOpt(sym, description, short)
		}
	end

	def self.version(version)
		@@toExec.push lambda {|s|
			s.version = version
		}
	end

	def self.description(str)
		@@toExec.push lambda {|s|
			s.description = str
		}
	end

	def self.credits(str)
		@@toExec.push lambda {|s|
			s.credits = str
		}
	end

	def self.usage(str)
		@@toExec.push lambda {|s|
			s.usage = str
		}
	end
	
	def self.name(str)
		@@toExec.push lambda {|s|
			s.name = str
		}
	end
	
	def self.run(klass)
		begin
			raise "ConsoleApplication.run must be passed a subclass of itself." unless klass<self
			$app = klass.new
			$app.start
		rescue SystemExit
			# Ignore
		rescue Exception
			if $app.nil? || $app.debug?
				puts "Error [#{$!.class}]: #{$!}"
				backtrace
			else
				puts "Error: {$!}"
			end
		end
	end
end

# Extend Symbol to make them sortable (for --help)
class Symbol
	def <=>(o)
		self.to_s<=>o.to_s
	end
end

# Print a colourised, formatted backtrace
def backtrace
	bt = "Backtrace:\n  " + $!.backtrace.join("\n  ").gsub(/(.+?):(.+?)(:|$)/, "#{Screen.colour['brown']}\\1:#{Screen.colour['yellow']}\\2\\3#{Screen.colour['normal']}")
	if nil==$app            
		puts bt         
	else                    
		puts bt if $app.debug?
	end
end