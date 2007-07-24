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

class ConsoleApplication
	attr_accessor :name, :mode, :credits
	
	def initialize
		@params = {}
		@paramData = {}
		@name = self.class.to_s
		@modes = []
		@mode = nil
		addMode('version', 'Displays version information', 'v')
		addMode('help', 'Displays this help')
	end
	def start
		begin
		current = nil
		ARGV.each {|p|
			if p[0] == ?- and @params.has_key?(p)
				p = @params[p]['canonicalName']
				@paramData[p] = {'list'=>[], 'occurrences'=>1}
				current = @params[p]['hasArgs'] ? p : nil
				if @params[p]['modeOpt']
					raise "Only one mode option may be specified. Mode is #{@mode}, #{p} was also specified." if @mode
					@mode = p[2,p.length]
				end
			elsif p[0] == ?-
				raise "No such option: #{p}"
			else
				if current
					@paramData[current]['list'].push p
				else
					raise "Invalid argument in this position: #{p}"
				end
			end
		}
		
		self.displayHelp if @mode == 'help'
		self.displayVersion if @mode == 'version'
		
		rescue Exception
			logError $!
			backtrace
		end
	end
	
	def displayHelp
		puts "Operational modes:"
		@modes.sort.each {|key|
			puts "#{key}#{@params[key]['short']?', -'+@params[key]['short']:''}"
			puts "        #{@params[key]['description']}"			
		}
		puts "Options:"
		@params.keys.sort.each {|key|
			next if !@params[key]['description'] or @params[key]['modeOpt']
			puts "#{key}#{@params[key]['short']?', -'+@params[key]['short']:''}"
			puts "        #{@params[key]['description']}"
		}
	end
	
	def displayVersion
		puts "#{@name} #{@version}"
		puts @credits
	end
	
	def addMode(long, helpText, short=nil)
		long = "--"+long
		@params[long] = {
			'canonicalName'=>long,
			'modeOpt'=>true,
			'hasArgs'=>false,
			'description'=>helpText,
			'short'=>short
		}
		@modes.push long
		addOptAlias('-'+short, long) if short
	end
	
	def addOptBoolean(long, helpText, short=nil)
		long = "--"+long
		@params[long] = {
			'canonicalName'=>long,
			'hasArgs'=>false,
			'description'=>helpText,
			'short'=>short
		}
		addOptAlias('-'+short, long) if short
	end
	
	def addOptAlias(from, to)
		to = @params[to]['canonicalName'] while @params[to]['canonicalName'] != to
		@params[from] = {
			'canonicalName'=>to
		}
	end
	
	def addOpt(long, helpText, short=nil)
		long = '--'+long
		@params[long] = {
			'canonicalName'=>long,
			'hasArgs'=>true,
			'description'=>helpText
		}
		addOptAlias('-'+short, long) if short
	end
	
	def logError(msg)
		puts "#{Screen.colour['cyan']}#{@name}#{Screen.colour['normal']}: #{Screen.colour['red']}Error:#{Screen.colour['normal']} #{msg}"
	end
	
	def logDebug(msg)
		puts "#{@name}: #{msg}" if debug?
	end
	
	def debug?
		true
	end
end