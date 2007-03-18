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