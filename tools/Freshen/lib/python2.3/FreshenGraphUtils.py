# XXXTODO: clean up arguments
# These functions have been hastily spun out to make the code easier to follow. They need cleaning up
# to fit better in their new home.
from CheckDependencies import *
from GuessLatest       import GuessLatest
def addProgramToGraph(graph, program, version, revision, ptype, url, upgradeDetails, neededBy=None, progressBar=None, installedPrograms=None, introducedBy=None, types=None):
	if program in graph.keys():
		return;
	if progressBar is not None:
		progressBar.inc()
	if neededBy is not None:
		if program in introducedBy:
			introducedBy[program]+=(neededBy,)
		else:
			introducedBy[program]=(neededBy,)
	upgradeDetails[program] = {'version': version, 'revision': revision, 'type': ptype, 'url': url}
	graph[program] = {}
	deps,sols = CheckDependencies(program, version, revision, ptype, url, types, mode='all', recursive=False)
	for s,d in zip(sols,deps):
		ver = s[1] + '-' + s[2]
		if s[0] not in installedPrograms:
			progressBar.size+=1
		if True or s[0] not in installedPrograms or (d['lover'] != '' and d['lover'] is not None and GuessLatest([d['lover'], GuessLatest(installedPrograms[s[0]])]) == d['lover']):
			graph[program][s[0]] = d
			addProgramToGraph(graph, s[0], s[1], s[2], s[3], s[4], upgradeDetails, program, progressBar, installedPrograms=installedPrograms, introducedBy=introducedBy, types=types)

# tsort' for the actual sorting. Keeps track of detected cycles in the global
# cycles.
def tsort2(item, graph, keys, ls, depth, parents, cycles=None):
	ind = ' '*depth
	if item is not None:
		k = item
		keys.remove(item)
	else:
		if len(keys) > 0:
			k = keys.pop()
		else:
			return
	for x in graph[k].keys():
		if x in keys:
			il = tsort2(x, graph, keys, [], depth+1, parents+[k], cycles=cycles)
			ls = il + ls
		else:
			if x in parents:
				ecyc = None
				for cyc in cycles:
					if ecyc is not None:
						if x in cyc or k in cyc:
							cycles.discard(cyc)
							cycles.discard(ecyc)
							cycles.add(cyc | ecyc)
							break
					elif x in cyc:
						cycles.discard(cyc)
						cycles.add(cyc | set([k]))
						ecyc = cyc
					elif k in cyc:
						cycles.discard(cyc)
						cycles.add(cyc | set([k]))
						ecyc = cyc
				#cycles = newcycles
				if ecyc is None:
					cycles.add(frozenset([x,k]))
	ls = [k] + ls
	return ls

def tsort(graph, cycles=None):
	keys = set(graph.keys())
	ls = []
	while len(keys)>0:
		il = tsort2(None, graph, keys, [], 1, [], cycles=cycles)
		ls = il + ls
	ls.reverse()
	return ls

def copyGraphFromNode(graph, newgraph, node, shallow=False, skipList=set(), installedPrograms=None):
	"""Copies the digraph starting from node from graph into newgraph, skipping entries in skipList."""
	newgraph[node] = graph[node]
	for e in newgraph[node].keys():
		if e in skipList:
			continue
		if e not in newgraph and (not shallow or (e not in installedPrograms or graph[node][e]['lover'] is not None and GuessLatest([graph[node][e]['lover'], GuessLatest(installedPrograms[e])]) == graph[node][e])):
			copyGraphFromNode(graph, newgraph, e, shallow=shallow, skipList=skipList, installedPrograms=installedPrograms)
