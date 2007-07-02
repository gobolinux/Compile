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

require 'tsort'
class DependencyHash < Hash
	include TSort
	alias tsort_each_node each_key
	attr_accessor :introducedBy
	def tsort_each_child(node, &block)
		if has_key?(node)
			fetch(node).each(&block)
		end
	end
end

def createDepHash(toup)
	# Local alias to save typing
	except = @config['except']
	ebc = @config['exceptButCompatible']
	introducedBy = {}
	dh = DependencyHash.new
	toup.each {|prog, ver|
		introducedBy[prog] = nil
		dh[prog] = getDependencies(prog, ver)
	}
	# This ensures that the complete dependencies of every program are included in the tree.
	begin
		mh = {}
		dh.each {|prog, deps|
			deps.each {|dep|
				next if dep.nil? || dep==""
				if !dh[dep]
					introducedBy[dep] = prog
					mh[dep] = getDependencies(dep, getNewestAvailableVersion(dep, prog))
				end
			}
		}
		dh.merge(mh)
	end while mh.size>0

	begin
		todel = []
		dh.each {|k, v|
			if except.include?(k)
				todel.push k
				next
			end
			v.each {|d|
				next if !d || d==""
				if except.include?(d) || !dh[d] # If this has an excluded or non-present dependency,
					todel.push k # drop it completely.
					break
				end
			}
		}
		todel.each {|item| # Delete them and go again
			#puts "Actually deleting #{item}"
			dh.delete item
		}
	end while todel.length>0
	ebc.each {|prog|
		if dh[prog]
			logVerbose "Note: removing #{prog} from updates on user assertion of compatibility"
			dh.delete prog
		end
	}
	dh['Xorg']-=['Mesa'] if dh['Xorg']
	dh['Mesa']-=['Xorg'] if dh['Mesa']
	dh.introducedBy = introducedBy
	return dh
end
