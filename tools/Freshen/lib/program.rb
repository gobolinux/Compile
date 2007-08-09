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

class Program
	attr_reader :name
	
	def initialize(name)
		@name = name.to_s
		@dir = '/Programs'
	end
	
	def to_s
		"#{@name}"
	end
	
	def current
		return @current if @current
		d = @dir
		if File.exist?(d + '/' + @name)
			v = `readlink -f #{d}/#{@name}/Current`
			v = v.slice(v.rindex('/')+1, v.length).strip
			@current = Version.new(v)
			# Throw in the revision if available
			if File.exist?(d+'/'+@name+'/'+@current.raw+'/Resources/Revision')
				(junk, rev) = IO.read(d+'/'+@name+'/'+@current.raw+'/Resources/Revision').strip.split('r')
				@current.revision = rev.to_i
			end
			@current
		else
			nil
		end
	end
	
	def installed_versions
		d = @dir
		if File.exist?(d + '/' + @name)
			versions = []
			Dir.foreach(d + '/' + @name) do |fn|
				case fn
				when '.', '..', 'Current', 'Settings', 'Variable'
					next
				else
					v = Version.new(fn)
					if File.exist?(d+'/'+@name+'/'+fn+'/Resources/Revision')
						(junk, rev) = IO.read(d+'/'+@name+'/'+fn+'/Resources/Revision').chomp.split('r')
						@current.revision = rev.to_i
					end
					versions.push v
				end
			end
			versions.sort
		else
			[]
		end
	end
	
	def installed?(version=nil)
		if version.nil?
			!current.nil?
		else
			installed_versions.include?(version)
		end
	end
end