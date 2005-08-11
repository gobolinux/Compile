#!/usr/bin/python

# (C) 2004 Andre Detsch. Released under the GNU GPL.

# GuessProgramCase
# Try to guess the case of a program based on the installed packages

### Changelog ########################################################

# 27/04/2004 - [detsch] Bugfix on when calling from command line
# 13/04/2004 - [detsch] Using ${goboPrograms}
# 10/04/2004 - [detsch] First version



def GuessProgramCase(program, version = None) :
	import os
	try :
		goboPrograms = os.environ['goboPrograms']
	except :
		goboPrograms = os.popen('. GoboPath; echo -n $goboPrograms').read()
	
	if version :
		if os.access(goboPrograms+'/'+program+'/'+version, os.R_OK) :
			return program
	else :
		if os.access(goboPrograms+'/'+program, os.R_OK) :
			return program

	p = program.lower()
	ds = [ d for d in os.listdir(goboPrograms) if d.lower() == p ]
	
	if len(ds) == 0 :
		return program
	elif len(ds) == 1 :
		return ds[0]
	
		
	for d in ds :
		if version :
			if os.access(goboPrograms+'/'+d+'/'+version, os.R_OK) :
				return d
		else :
			if os.access(goboPrograms+'/'+d, os.R_OK) :
				return d
	
	return program


if __name__ == '__main__' :
	import sys
	if len(sys.argv) < 2 :
		print "Usage: %s program_name [version]"%sys.argv[0]
	elif len(sys.argv) == 2 :
		print GuessProgramCase(sys.argv[1])
	else :
		print GuessProgramCase(sys.argv[1], sys.argv[2])

