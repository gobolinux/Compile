# Copyright (C) 2007 Michael Homer
# Released under the GNU GPL

require 'tsort'
class DependencyHash < Hash
	include TSort
	alias tsort_each_node each_key
	def tsort_each_child(node, &block)
		if has_key?(node)
			fetch(node).each(&block)
		end
	end
end

def createDepHash(toup)
	dh = DependencyHash.new
	toup.each {|prog, ver|
		dh[prog] = getDependencies(prog, ver)
		# SPECIAL CASE: circular dependency
		if prog=='Xorg' and dh[prog].include?('Mesa')
			#self.logError "Circular dependency of Xorg on Mesa removed from tree"
			dh[prog]-= ['Mesa']
		end
	}
	
	# This ensures that the complete dependencies of every program are included in the tree.
	begin
		mh = {}
		dh.each {|k, v|
			v.each {|d|
				next if d.nil? || d==""
				if !dh[d]
					mh[d] = getDependencies(d, getNewestAvailableVersion(d, k))
				end
			}
		}
		dh.merge(mh)
	end while mh.size>0
	except = @config['except']
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
	return dh
end
