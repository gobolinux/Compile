#!/bin/sh

if [ "$1" = "--clean" ]
then
   rm -fv src/*.h
   rm -fv Makefile.in
   rm -fv configure.ac
   rm -fvr autom4te.cache
   exit 0
fi

scripts/AllHeaders
aclocal
autoconf
