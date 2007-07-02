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

# Implements "merge" mixin on Hash and Array, global simple method to recursively combine
# Note, not the same as Hash#update, in that it is recursive
# Also includes prettier to_s methods and a WarningHash class

class Array
	def to_s
		'Array (' + self.join(', ') + ')'
	end
	def merge(array)
		self.concat array
	end
end
class Hash
	def to_s
		rs = "Hash (\n"
		self.each do |key, value|
			rs += "  #{key} => #{value}\n"
		end
		rs + ')'
	end
	def merge(hash)
		return if !hash.is_a?(Hash)
		hash.each do |key, value|
			if self.member?(key)
				j = self[key].dup
				j.merge value
				self[key] = j
			else
				self.store key, value
			end
		end
	end
end

# This is so I get around to replacing all those nasty deprecated hashes
# Requires $app to implement GoboLogConsole
class WarningHash < Hash
	def [](x)
		caller[0] =~ /.*\/(.+?):(\d+):/
		$app.logDebug "Warning: use of deprecated hash in #{$1} line #{$2}."
		super x
	end
end