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

class CollectionRepository < Repository
	@repositories = []
	
	def add(rep)
		@repositories.push rep
	end
	
	def include?(prog, ver=nil)
		@repositories.each do |rep|
			return true if rep.include?(prog, ver)
		end
	end
	
	def meet_dependency?(dep)
		@repositories.each do |rep|
			return true if rep.meet_dependency?(dep)
		end
	end
	
	def [](name)
		vers = []
		@repositories.each do |rep|
			vers |= rep[name]
		end
		vers.sort
	end
end