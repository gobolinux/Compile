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

class ProgramRepository < Repository
	
	@programs = Hash.new
	
	def new(dir="/Programs")
		super
		@dir = dir
	end
	
	def include?(prog, ver=nil)
		if ver
			File.exist("#{@dir}/#{prog}/#{ver.raw}")
		else
			File.exist("#{@dir}/#{prog}")
		end
	end
	
	def meet_dependency?(dep)
		raise NotImplementedError
	end
	
	def all
		@programs = Hash.new
		Dir.foreach(@dir) do |name|
			next if name == '.' or name == '..'
			@programs[name] = Program.new name
		end
		@programs
	end
	
	def [](name)
		@programs[name] = Program.new name
		@programs[name]
	end
end