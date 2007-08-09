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

class Version
	include Comparable
	
	attr_reader :versionTokens, :version, :revision, :type, :raw
	attr_writer :revision
	
	@@useRevisions = true
	
	def initialize(s)
		@raw = s
		(@version, @revision) = s.split('-r')
		@revision = @revision.nil? ? 1 : @revision.to_i
		(@version, @type) = @version.split(/(?=alpha|beta|pre|rc)/)
		@versionTokens = @version.split(/ [._] | (\d+)(?=[a-z]) | ([a-z]+)(?=\d) /x).delete_if {|y| y==""}
	end
	
	def to_s
		"#{@version}#{@type}-r#{@revision}"
	end
	
	def <=>(other)
		@versionTokens.zip(other.versionTokens) {|m, o|
			#puts "comparing #{m} and #{o}"
			if o.nil?
				# If the other version has run out of tokens, keep going as long as we're
				# all zeroes.
				case m
				when '0'
					# Do nothing this time, so 4.0.0 == 4.0
					next
				when /^\d+$/, /^[a-z]$/
					return 1
				else
					break
				end
			end
			r = compareTokens(m,o)
			return r if r
		}
		# Exhausted our own tokens: do they have more? If they're zeroes, ignore.
		if other.versionTokens.length > @versionTokens.length
			@versionTokens.length.upto(other.versionTokens.length) {|i|
				case other.versionTokens[i]
				when '0'
					# Do nothing: 4.0.0 == 4.0
				when /^\d+$/, 'a'..'z'
					return -1
				end
			}
		end
		otherType = other.type
		if !@type.nil?
			return -1 if otherType.nil?
			return (@type <=> otherType)
		elsif otherType
			return 1
		end
		# At this point, the version is equal so compare revisions
		return (@revision <=> other.revision) if @@useRevisions
		return 0
	end
	
	private
	def compareTokens(m,o)
		return 1 if m.to_i.to_s == m && o.to_i.to_s != o
		return -1 if o.to_i.to_s == o && m.to_i.to_s != m
		if m.to_i.to_s == m && o.to_i.to_s == o
			return 1 if m.to_i > o.to_i
			return -1 if m.to_i < o.to_i
		end
		return 1 if m > o
		return -1 if m < o
		nil
	end
	
	public
	def self.useRevisions=(x)
		@@useRevisions = x
	end
end