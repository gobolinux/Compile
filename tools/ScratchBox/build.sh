#!/bin/zsh

#
# Simple script used to unalias make=ColorMake and start
# compiling the world.
#

alias make && unalias make
[ ! -f .config ] && make menuconfig || make
