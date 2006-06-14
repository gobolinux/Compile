#!/bin/sh
#   bash_completion - some programmable completion functions for bash 2.05a
#
#   $Id: completion.bash,v 1.3 2006-06-14 23:45:18 drmoose Exp $
#
#   Copyright (C) Ian Macdonald <ian@caliban.org>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2, or (at your option)
#   any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software Foundation,
#   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

# RELEASE: 20020331

[ -n "$DEBUG" ] && set -v || set +v

# Alter the following to reflect the location of this file
#
[ -z "$BASH_COMPLETION" ] && declare -r BASH_COMPLETION="${goboPrefix}/System/Settings/completion.bash"
[ -z "$BASH_COMPLETION_DIR" ] && \
	declare -r BASH_COMPLETION_DIR="${goboPrefix}/System/Settings/completion"

# Set a couple of useful vars
#
OS=$( uname -s )
RELEASE=$( uname -r )

# Turn on extended globbing and programmable completion
shopt -s extglob progcomp

# A lot of the following one-liners were taken directly from the
# completion examples provided with the bash 2.04 source distribution

# Make directory commands see only directories
complete -d pushd

# The following section lists completions that are redefined later
# Do NOT break these over multiple lines.
#
# START exclude -- do NOT remove this line
complete -f -X '!*.bz2' bunzip2 bzcat bzcmp bzdiff bzegrep bzfgrep bzgrep
complete -f -X '!*.@(zip|ZIP|jar|JAR|exe|EXE|pk3)' unzip
complete -f -X '*.Z' compress znew
complete -f -X '!*.@(Z|gz|tgz|Gz)' gunzip zcmp zdiff zcat zegrep zfgrep zgrep zless zmore
complete -f -X '!*.Z' uncompress
complete -f -X '!*.@(gif|jp?(e)g|tif?(f)|png|p[bgp]m|bmp|xpm|ico|GIF|JPG|JP?(E)G|TIF?(F)|PNG|P[BGP]M|BMP|XPM|ICO)' ee display
complete -f -X '!*.@(gif|jp?(e)g|tif?(f)|png|p[bgp]m|bmp|xpm|GIF|JPG|JP?(E)G|TIF?(F)|PNG|P[BGP]M|BMP|XPM)' xv qiv
complete -f -X '!*.@(@(?(e)ps|?(E)PS|pdf|PDF)?(.gz|.GZ))' gv ggv
complete -f -X '!*.@(dvi|DVI)?(.@(gz|Z|bz2))' xdvi
complete -f -X '!*.@(dvi|DVI)' dvips dviselect dvitype
complete -f -X '!*.@(pdf|PDF)' acroread xpdf
complete -f -X '!*.texi*' makeinfo texi2dvi texi2html
complete -f -X '!*.@(tex|TEX)' tex latex slitex jadetex pdfjadetex pdftex pdflatex
complete -f -X '!*.@(mp3|MP3)' mpg123
complete -f -X '!*.@(mpg|mpeg|avi|asf|vob|bin|vcd|ps|pes|fli|viv|rm|ram|yuv|mov|wmv)' mplayer
complete -f -X '!*.@(avi|asf)' aviplay
complete -f -X '!*.@(rm|ram|smi?(l))' realplay
complete -f -X '!*.@(mpg|mpeg|avi|mov)' xanim
complete -f -X '!*.@(ogg|OGG)' ogg123
complete -f -X '!*.@(mp3|MP3|ogg|OGG|pls)' xmms gqmpeg freeamp
complete -f -X '!*.fig' xfig
complete -f -X '!*.@(mid?(i))' timidity playmidi
complete -f -X '!*.@(exe|EXE|com|COM)' wine
# FINISH exclude -- do not remove this line

# start of section containing compspecs that can be handled within bash

# user commands see only users
complete -u finger su usermod userdel passwd chage write talk chfn

# group commands see only groups
complete -g groupmod groupdel

# bg completes with stopped jobs
complete -A stopped -P '%' bg

# other job commands
complete -j -P '%' fg jobs disown

# readonly and unset complete with shell variables
complete -v readonly unset

# set completes with set options
complete -A setopt set

# shopt completes with shopt options
complete -A shopt shopt

# helptopics
complete -A helptopic help

# unalias completes with aliases
complete -a unalias

# bind completes with readline bindings (make this more intelligent)
complete -A binding bind

# type completes with commands
complete -c command type

# start of section containing completion functions called by other functions

# This function checks whether we have a given programs on the system.
# No need for bulky functions in memory if we don't.
#
have()
{
	unset -v have
	type $1 &> /dev/null
	[ $? -eq 0 ] && have="yes"
}

# This function performs file and directory completion. It's better than
# simply using 'compgen -f', because it honours spaces in filenames
#
_filedir()
{
	local IFS=$'\t\n'

	_expand || return 0

	if [ "$1" = -d ]; then
		COMPREPLY=( ${COMPREPLY[@]} $( compgen -d -- $cur ) )
		return 0
	fi
	COMPREPLY=( ${COMPREPLY[@]} $( eval compgen -f -- \"$cur\" ) )
}

# This function completes on signal names
#
_signals()
{
	local i

	# standard signal completion is rather braindead, so we need
	# to hack around to get what we want here, which is to
	# complete on a dash, followed by the signal name minus
	# the SIG prefix
	COMPREPLY=( $( compgen -A signal SIG${cur#-} ))
	for (( i=0; i < ${#COMPREPLY[@]}; i++ )); do
		COMPREPLY[i]=-${COMPREPLY[i]#SIG}
	done
}

# This function expands tildes in pathnames
#
_expand()
{
	[ "$cur" != "${cur%\\}" ] && cur="$cur"'\'
	
	# expand ~username type directory specifications
	if [[ "$cur" == \~*/* ]]; then
		eval cur=$cur
	elif [[ "$cur" == \~* ]]; then
		cur=${cur#\~}
		COMPREPLY=( $( compgen -P '~' -u $cur ) )
		return ${#COMPREPLY[@]}
	fi
}

# start of section containing completion functions for bash built-ins

# bash alias completion
#
_alias()
{
	local cur

	COMPREPLY=()
	cur=${COMP_WORDS[$COMP_CWORD]};

	case "$COMP_LINE" in
	*[^=])
		COMPREPLY=( $( compgen -A alias -S '=' -- $cur ) )
		;;
	*=)
		COMPREPLY=( "$( alias ${cur%=} 2>/dev/null | \
			     sed -e 's|^alias '$cur'\(.*\)$|\1|' )" )
		;;
	esac
}
complete -F _alias alias

# bash export completion
#
_export()
{
	local cur

	COMPREPLY=()
	cur=${COMP_WORDS[$COMP_CWORD]};

	case "$COMP_LINE" in
	*[^=])
		COMPREPLY=( $( compgen -v -S '=' -- $cur ) )
		;;
	*=)
		COMPREPLY=( $( eval echo $`echo ${cur%=}` ) )
		;;
	esac
}
complete -F _export -o default export

# bash shell function completion
#
_function()
{
	local cur prev

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	if [[ $1 == @(declare|typeset) ]]; then
		if [ "$prev" = -f ]; then
			COMPREPLY=( $( compgen -A function -- $cur ) )
		elif [[ "$cur" == -* ]]; then
			COMPREPLY=( $( compgen -W '-a -f -F -i -r -x -p' -- \
				       $cur ) )
		fi
	elif [ $COMP_CWORD -eq 1 ]; then
		COMPREPLY=( $( compgen -A function -- $cur ) )
	else
		COMPREPLY=( "() $( type -- ${COMP_WORDS[1]} | sed -e 1,2d )" )
	fi
}
complete -F _function function declare typeset

# start of section containing completion functions for external programs

# chown(1) completion
#
_chown()
{
	local cur prev user group i

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	# do not attempt completion if we're specifying an option
	[[ "$cur" == -* ]] && return 0

	# first parameter on line or first since an option?
	if [ $COMP_CWORD -eq 1 ] || [[ "$prev" == -* ]]; then
		if [[ "$cur" == [a-zA-Z]*[.:]* ]]; then
			user=${cur%%?(\\)[.:]*}
			group=${cur#*[.:]}
			COMPREPLY=( $( compgen -P $user':' -g -- $group ) )
		else
			COMPREPLY=( $( compgen -S ':' -u $cur ) )
		fi
	fi

	return 0
}
complete -F _chown -o default chown

# chgrp(1) completion
#
_chgrp()
{
	local cur prev

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	# do not attempt completion if we're specifying an option
	[[ "$cur" == -* ]] && return 0

	# first parameter on line or first since an option?
	if [ $COMP_CWORD -eq 1 ] || [[ "$prev" == -* ]]; then
		_expand || return 0
		COMPREPLY=( $( compgen -g $cur ) )
	fi

	return 0
}
complete -F _chgrp -o default chgrp

# umount(8) completion. This relies on the mount point being the third
# space-delimited field in the output of mount(8)
#
_umount()
{
	local cur

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}

	COMPREPLY=( $( mount | cut -d' ' -f 3 | grep ^$cur ) )

	return 0
}
complete -F _umount -o filenames umount

# mount(8) completion. This will pull a list of possible mounts out of
# /etc/fstab, unless the word being completed contains a ':', which
# would indicate the specification of an NFS server. In that case, we
# query the server for a list of all available exports and complete on
# that instead.
#
_mount()
{       local cur i sm

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}
	for i in {,/usr}/{,s}bin/showmount; do [ -x $i ] && sm=$i && break; done

	if [ -n "$sm" ] && [[ "$cur" == *:* ]]; then
		COMPREPLY=( $( $sm -e ${cur%%:*} | sed 1d | \
			       grep ^${cur#*:} | awk '{print $1}' ) )
	else
		COMPREPLY=( $( awk '{if ($2 ~ /\//) print $2}' /etc/fstab | \
			       grep ^$cur ) )
	fi

	return 0
}
complete -F _mount -o default mount

# Linux rmmod(1) completion. This completes on a list of all currently
# installed kernel modules.
#
[ $OS = Linux ] &&
_rmmod()
{
	local cur

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}

	COMPREPLY=( $( /sbin/lsmod | \
		       awk '{if (NR != 1 && $1 ~ /^'$cur'/) print $1}' ) )
	return 0
}
[ $OS = Linux ] && complete -F _rmmod rmmod

# Linux insmod(8) & modprobe(8) completion. This completes on a list of all
# available modules for the version of the kernel currently running.
#
[ $OS = Linux ] &&
_insmod()
{
	local cur prev modpath

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}
	modpath=/lib/modules/`uname -r`

	# behave like lsmod for modprobe -r
	if [ $1 = "modprobe" ] &&
	   [ "${COMP_WORDS[1]}" = "-r" ]; then
		COMPREPLY=( $( /sbin/lsmod | \
				awk '{if (NR != 1 && $1 ~ /^'$cur'/) print $1}' ) )
		return 0
	fi

	# do filename completion if we're giving a path to a module
	if [[ "$cur" == /* ]]; then
		_filedir
		return 0
	fi

	if [ $COMP_CWORD -gt 1 ]; then
		# do module parameter completion
		COMPREPLY=( `/sbin/modinfo -p ${COMP_WORDS[1]} 2>/dev/null | \
		       awk '{if ($1 ~ /^parm:/ && $2 ~ /^'$cur'/) { print $2 } \
			else if ($1 !~ /:/ && $1 ~ /^'$cur'/) { print $1 }}' ` )
	elif [ -r $modpath -a -x $modpath ]; then
		# do module name completion
		COMPREPLY=( $( \ls -R $modpath | \
                                sed -ne 's/^\('$cur'.*\)\.o\(\|.gz\)$/\1/p') )
	else 
		_filedir
	fi

	return 0
}
[ $OS = Linux ] && complete -F _insmod -o filenames insmod modprobe

# man(1) completion. This is Linux and Darwin specific, in that
# 'man <section> <page>' is the expected syntax.
#
[ $OS = Linux -o $OS = Darwin ] &&
_man()
{
	local cur prev sect manpath OS
	manAlias=`alias man | cut -d\' -f2`

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	_expand || return 0

	# default completion if parameter contains /
	if [[ "$cur" == */* ]]; then
		_filedir
		alias man=$manAlias
		return 0
	fi

	OS=$( uname -s )
	if [ $OS = Linux ]; then
		manpath=$( manpath 2>/dev/null || man --path )
	else
		manpath=$MANPATH
	fi

	if [ -z "$manpath" ]; then
		COMPREPLY=( $( compgen -c -- $cur ) )
		alias man=$manAlias
		return 0
	fi

	# determine manual section to search
	[[ "$prev" == [0-9ln] ]] && sect=$prev || sect='?'

	manpath=$manpath:
	if [ -n "$cur" ]; then
		manpath="${manpath//://man$sect/$cur* }"
	else
		manpath="${manpath//://man$sect/ }"
	fi
		
	# redirect stderr for when path doesn't exist
	COMPREPLY=( $( eval \\ls "$manpath" 2>/dev/null ) )
	# weed out directory path names and paths to man pages
	COMPREPLY=( ${COMPREPLY[@]##*/?(:)} )
	# strip suffix from man pages
	COMPREPLY=( ${COMPREPLY[@]%.@(gz|bz2)} )
	COMPREPLY=( ${COMPREPLY[@]%.*} )

	if [[ "$prev" != [0-9ln] ]]; then
		COMPREPLY=( ${COMPREPLY[@]} $( compgen -d -- $cur ) \
			    $( compgen -f -X '!*.[0-9ln]' -- $cur ) )
	fi

	alias man=$manAlias
	return 0
}
[ $OS = Linux -o $OS = Darwin ] && complete -F _man -o filenames man

# kill(1) completion
#
_kill()
{
	local cur

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}

	if [ $COMP_CWORD -eq 1 ] && [[ "$cur" == -* ]]; then
		# return list of available signals
		_signals
	else
		# return list of available PIDs
		COMPREPLY=( $( \ls /proc | grep '^[0-9]\+'| grep ^$cur ) )
	fi
}
complete -F _kill kill

# Linux and FreeBSD killall(1) completion.
#
[ $OS = Linux -o $OS = FreeBSD ] &&
_killall()
{
	local cur

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}

	if [ $COMP_CWORD -eq 1 ] && [[ "$cur" == -* ]]; then
		_signals
	else
		COMPREPLY=( $( compgen -W '$( ps axo ucomm | sed 1d )' \
			       -- $cur ) )
	fi

	return 0
}
[ $OS = Linux -o $OS = FreeBSD ] && complete -F _killall killall

# GNU find(1) completion. This makes heavy use of ksh style extended
# globs and contains Linux specific code for completing the parameter
# to the -fstype option.
#
_find()
{
	local cur prev

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	case "$prev" in
	-@(max|min)depth)
		COMPREPLY=( $( compgen -W '0 1 2 3 4 5 6 7 8 9' -- $cur ) )
		return 0
		;;
	-?(a)newer|-fls|-fprint?(0|f)|-?(i)?(l)name)
		_filedir
		return 0
		;;
	-fstype)
		# this is highly non-portable
		COMPREPLY=( $( cut -d$'\t' -f 2 /proc/filesystems | grep ^$cur))
		return 0
		;;
	-gid)
		COMPREPLY=( ` awk 'BEGIN {FS=":"} \
				{if ($3 ~ /^'$ncur'/) print $3}' /etc/group ` )
		return 0
		;;
	-group)
		COMPREPLY=( $( compgen -g -- $cur ) )
		return 0
		;;
	-?(x)type)
		COMPREPLY=( $( compgen -W 'b c d p f l s' -- $cur ) )
		return 0
		;;
	-uid)
		COMPREPLY=( ` awk 'BEGIN {FS=":"} \
				{if ($3 ~ /^'$ncur'/) print $3}' /etc/passwd ` )
		return 0
		;;
	-user)
		COMPREPLY=( $( compgen -u -- $cur ) )
		return 0
		;;
	-[acm]min|-[acm]time|-?(i)?(l)name|-inum|-?(i)path|-?(i)regex| \
	-links|-perm|-size|-used|-exec|-ok|-printf)
		# do nothing, just wait for a parameter to be given
		return 0
		;;
	esac

	_expand || return 0

	# handle case where first parameter is not a dash option
	if [ $COMP_CWORD -eq 1 ] && [[ "$cur" != -* ]]; then
		_filedir -d
		return 0
	fi

	# complete using basic options
	COMPREPLY=( $( compgen -W '-daystart -depth -follow -help -maxdepth \
			-mindepth -mount -noleaf -version -xdev -amin -anewer \
			-atime -cmin -cnewer -ctime -empty -false -fstype \
			-gid -group -ilname -iname -inum -ipath -iregex \
			-links -lname -mmin -mtime -name -newer -nouser \
			-nogroup -perm -regex -size -true -type -uid -used \
			-user -xtype -exec -fls -fprint -fprint0 -fprintf -ok \
			-print -print0 -printf -prune -ls' -- $cur ) )

	# this removes any options from the list of completions that have
	# already been specified somewhere on the command line.
	COMPREPLY=( $( echo "${COMP_WORDS[@]}" | \
		       (while read -d ' ' i; do
			    [ "$i" == "" ] && continue
			    # flatten array with spaces on either side,
			    # otherwise we cannot grep on word boundaries of
			    # first and last word
			    COMPREPLY=" ${COMPREPLY[@]} "
			    # remove word from list of completions
			    COMPREPLY=( ${COMPREPLY/ ${i%% *} / } )
			done
			echo ${COMPREPLY[@]})
		  ) )
	
	return 0
}
complete -F _find -o filenames find

# Linux ifconfig(8) completion
#
[ $OS = Linux ] &&
_ifconfig()
{
	local cur

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}

	case "${COMP_WORDS[1]}" in
	-|*[0-9]*)
		COMPREPLY=( $( compgen -W '-a up down arp promisc allmulti \
					   metric mtu dstaddr netmask add del \
					   tunnel irq io_addr mem_start media \
					   broadcast pointopoint hw multicast \
					   address txqueuelen' $cur ))
		COMPREPLY=( $( echo " ${COMP_WORDS[@]}" | \
			       (while read -d ' ' i; do
				   [ "$i" == "" ] && continue
				   # flatten array with spaces on either side,
				   # otherwise we cannot grep on word
				   # boundaries of first and last word
				   COMPREPLY=" ${COMPREPLY[@]} "
				   # remove word from list of completions
				   COMPREPLY=( ${COMPREPLY/ $i / } )
				done
				echo ${COMPREPLY[@]})
			  ) )
		return 0
		;;
	esac

	COMPREPLY=( $( /sbin/ifconfig -a | \
		       sed -ne 's|^\('$cur'[^ ]*\).*$|\1|p' ))
}
[ $OS = Linux ] && complete -F _ifconfig ifconfig

# RedHat & Debian Linux if{up,down} completion
#

# cvs(1) completion
#
# chsh(1) completion
#
_chsh()
{
	local cur prev

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	if [ "$prev" = "-s" ]; then
		COMPREPLY=( $( chsh -l | grep ^$cur ) )
	else
		COMPREPLY=( $( compgen -u -- $cur ) )
	fi

	return 0
}
complete -F _chsh chsh

# chkconfig(8) completion
#
have chkconfig &&
_chkconfig()
{
	local cur prev

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	if [ $COMP_CWORD -eq 1 ]; then
		COMPREPLY=( $( compgen -W '--list --add --del --level' -- \
			       $cur ) )
		return 0
	fi

	if [ $COMP_CWORD -eq 4 ]; then
		COMPREPLY=( $( compgen -W 'on off reset' -- $cur ) )
		return 0
	fi

	case "$prev" in
	@([1-6]|--@(list|add|del)))
		COMPREPLY=( $( compgen -W "`(cd /etc/rc.d/init.d; echo *)`" \
			       -- $cur) )

		return 0
		;;
	--level)
		COMPREPLY=( $( compgen -W '1 2 3 4 5 6' -- $cur ) )
		return 0
		;;
	esac

	return 0
}
[ "$have" ] && complete -F _chkconfig chkconfig

# This function performs host completion based on ssh's known_hosts files,
# defaulting to standard host completion if they don't exist.
#
_known_hosts()
{
	local cur user suffix
	local -a kh

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}
	[ "$1" = -c ] && suffix=':'
	[[ $cur == *@* ]] && user=${cur%@*}@ && cur=${cur#*@}
	kh=()

	[ -r /etc/known_hosts ]    && kh[0]=/etc/known_hosts
	[ -r /etc/known_hosts2 ]   && kh[1]=/etc/known_hosts2
	[ -r ~/.ssh/known_hosts ]  && kh[2]=~/.ssh/known_hosts
	[ -r ~/.ssh/known_hosts2 ] && kh[3]=~/.ssh/known_hosts2

	# If we have known_hosts files to use
	if [ ${#kh[@]} -gt 0 ]; then
	    # Escape slashes and dots in paths for awk
	    cur=${cur//\//\\\/}
	    cur=${cur//\./\\\.}
	    if [[ "$cur" == [0-9]*.* ]]; then
		# Digits followed by a dot - just search for that
		cur="^$cur.*"
	    elif [[ "$cur" == [0-9]* ]]; then
		# Digits followed by no dot - search for digits followed
		# by a dot
		cur="^$cur.*\."
	    elif [ -z "$cur" ]; then
		# A blank - search for a dot or an alpha character
		cur="[a-z.]"
	    else
		cur="^$cur"
	    fi
	    # FS needs to look for a comma separated list
	    COMPREPLY=( ` awk 'BEGIN {FS=","}
				{for (i=1; i<=2; ++i) { \
				       gsub(" .*$", "", $i); \
				       if ($i ~ /'$cur'/) {print $i} \
				}}' ${kh[@]} ` )
	    for (( i=0; i < ${#COMPREPLY[@]}; i++ )); do
		COMPREPLY[i]=$user${COMPREPLY[i]}$suffix
	    done
	else
	    # Just do normal hostname completion
	    COMPREPLY=( $( compgen -A hostname -S "$suffix" -- $cur ) )
	fi

	return 0
}
complete -F _known_hosts traceroute ping fping telnet host nslookup rsh \
			 rlogin ftp

# ssh(1) completion
#
have ssh &&
_ssh()
{
	local cur prev

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	case "$prev" in
	-*c)
	    COMPREPLY=( $( compgen -W 'blowfish 3des 3des-cbc blowfish-cbc \
			   arcfour cast128-cbc' -- $cur ) )
	    ;;
	-*l)
	    COMPREPLY=( $( compgen -u -- $cur ) )
	    ;;
	*)
	    _known_hosts
	    [ $COMP_CWORD -eq 1 ] || \
		COMPREPLY=( ${COMPREPLY[@]} $( compgen -c -- $cur ) )
	esac

	return 0
}
[ "$have" ] && shopt -u hostcomplete && complete -F _ssh ssh slogin sftp

# scp(1) completion
#
have scp &&
_scp()
{
	local cur userhost path

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}

	_expand || return 0

	if [[ "$cur" == *:* ]]; then
		# remove backslash escape from :
		cur=${cur/\\:/:}
		userhost=${cur%%?(\\):*}
		path=${cur#*:}
		if [ -z "$path" ]; then
			# default to home dir of specified user on remote host
			path=$( ssh -o 'Batchmode yes' $userhost pwd )
		fi
		COMPREPLY=( ` compgen -P "$userhost:" \
			       -W "$(echo $( ssh -o 'Batchmode yes' $userhost \
					    compgen -f -- $path ))" ` )
		return 0
	fi

	[[ "$cur" == */* ]] || _known_hosts -c
	_filedir

	return 0
}
[ "$have" ] && complete -o filenames -F _scp scp

# Linux route(8) completion
#
[ $OS = Linux ] &&
_route()
{
	local cur prev

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	if [ "$prev" = dev ]; then
	    COMPREPLY=( $( ifconfig -a | sed -ne 's|^\('$cur'[^ ]*\).*$|\1|p' ))
	    return 0
	fi

	COMPREPLY=( $( compgen -W 'add del -host -net netmask metric mss \
				   window irtt reject mod dyn reinstate dev' \
				   -- $cur ) )

	COMPREPLY=( $( echo " ${COMP_WORDS[@]}" | \
		       (while read -d ' ' i; do
			   [ "$i" == "" ] && continue
			   # flatten array with spaces on either side,
			   # otherwise we cannot grep on word
			   # boundaries of first and last word
			   COMPREPLY=" ${COMPREPLY[@]} "
			   # remove word from list of completions
			   COMPREPLY=( ${COMPREPLY/ $i / } )
			done
		       echo ${COMPREPLY[@]})
		  ) )
	return 0
}
[ $OS = Linux ] && complete -F _route route

# GNU make(1) completion (adapted from the example supplied with the bash 2.04
# source code)
#
have make &&
_make()
{
	local mdef makef gcmd cur prev i

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	# if prev argument is -f, return possible filename completions.
	# we could be a little smarter here and return matches against
	# `makefile Makefile *.mk', whatever exists
	if [[ "$prev" == -*f ]]; then
		_filedir
		return 0
	fi

	# check for a long option
	if [[ "$cur" == --* ]]; then
		_longopt $1
		return 0
	fi

	# if we want an option, return the possible posix options
	if [[ "$cur" == -* ]]; then
		COMPREPLY=( $( compgen -W '-e -f -i -k -n -p -q -r -S -s -t' \
			       -- $cur ) )
		return 0
	fi

	# make reads `makefile' before `Makefile'
	if [ -f makefile ]; then
		mdef=makefile
	elif [ -f Makefile ]; then
		mdef=Makefile
	else
		mdef=*.mk	       # local convention
	fi

	# before we scan for targets, see if a makefile name was specified
	# with -f
	for (( i=0; i < ${#COMP_WORDS[@]}; i++ )); do
		if [[ ${COMP_WORDS[i]} == -*f ]]; then
			eval makef=${COMP_WORDS[i+1]} # eval for tilde expansion
			break
		fi
	done

	[ -z "$makef" ] && makef=$mdef

	# if we have a partial word to complete, restrict completions to
	# matches of that word
	[ -n "$2" ] && gcmd='grep "^$2"' || gcmd=cat

	# if we don't want to use *.mk, we can take out the cat and use
	# test -f $makef and input redirection  
	COMPREPLY=( $( cat $makef 2>/dev/null | \
		       awk 'BEGIN {FS=":"} /^[^.#	][^=]*:/ {print $1}' | \
		       eval $gcmd ) )

	# default to filename completion if all else failed
	[ ${#COMPREPLY[@]} -eq 0 ] && _filedir

	return 0
}
[ "$have" ] && complete -F _make -X '+($*|*.[cho])' -o filenames make gmake pmake

# Red Hat Linux service completion. This completes on a list of all available
# service scripts in the SysV init.d directory, followed by that script's
# available commands
#
# GNU tar(1) completion
#
_tar()
{
	local cur

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}

	if [ $COMP_CWORD = 1 ]; then
		COMPREPLY=( $( compgen -W 'c t x u r d A' -- $cur ) )
		return 0
	fi

	case "${COMP_WORDS[1]}" in
	c*f)
		_filedir
		;;
	+([^Izjy])f)
		COMPREPLY=( $( compgen -d -- $cur ) \
			    $( compgen -f -X '!*.tar' -- $cur ) )
		;;
	*z*f)
		COMPREPLY=( $( compgen -d -- $cur ) \
			    $( compgen -f -X '!*.t?(ar.)gz' -- $cur ) )
		;;
	*[Ijy]*f)
		COMPREPLY=( $( compgen -d -- $cur ) \
			    $( compgen -f -X '!*.tar.bz2' -- $cur ) )
		;;
	*)
		_filedir
		;;
		
	esac

	return 0
}
complete -F _tar -o filenames tar

# jar(1) completion
#
have jar &&
_jar()
{
	local cur

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}

	if [ $COMP_CWORD = 1 ]; then
		COMPREPLY=( $( compgen -W 'c t x u' -- $cur ) )
		return 0
	fi

	case "${COMP_WORDS[1]}" in
		c*f)
			_filedir
			;;
		*f)
			COMPREPLY=( $( compgen -d -- $cur ) \
				    $( compgen -f -X '!*.jar' -- $cur ) )
			;;
		*)
			_filedir
			;;
	esac
}
[ "$have" ] && complete -F _jar -o filenames jar

# Linux iptables(8) completion
#
have iptables &&
_iptables()
{
	local cur prev table chain

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]} 
	prev=${COMP_WORDS[COMP_CWORD-1]}
	chain='s/^Chain \([^ ]\+\).*$/\1/p'

	if [[ $COMP_LINE == *-t\ *filter* ]]; then
		table="-t filter"
	elif [[ $COMP_LINE == *-t\ *nat* ]]; then
		table="-t nat"
	elif [[ $COMP_LINE == *-t\ *mangle* ]]; then
		table="-t mangle"
	fi

	case "$prev" in
	-*[AIDPFXL])
		COMPREPLY=( $( compgen -W '`iptables $table -nL | \
			    sed -ne "s/^Chain \([^ ]\+\).*$/\1/p"`' -- $cur ) )
		;;
	-*t)
		COMPREPLY=( $( compgen -W 'nat filter mangle' -- $cur ) )
		;;
	-j)
		if [ "$table" = "-t filter" -o "$table" = "" ]; then
		    COMPREPLY=( $( compgen -W 'ACCEPT DROP LOG ULOG REJECT \
		    `iptables $table -nL | sed -ne "$chain" \
		    -e "s/INPUT|OUTPUT|FORWARD|PREROUTING|POSTROUTING//"`' -- \
		    $cur ) )
		elif [ "$table" = "-t nat" ]; then
		    COMPREPLY=( $( compgen -W 'ACCEPT DROP LOG ULOG REJECT \
		    MIRROR SNAT DNAT MASQUERADE `iptables $table -nL | \
		    sed -ne "$chain" -e "s/OUTPUT|PREROUTING|POSTROUTING//"`' \
		    -- $cur ) )
		elif [ "$table" = "-t mangle" ]; then
		    COMPREPLY=( $( compgen -W 'ACCEPT DROP LOG ULOG REJECT \
		    MARK TOS `iptables $table -nL | sed -ne "$chain" \
		    -e "s/INPUT|OUTPUT|FORWARD|PREROUTING|POSTROUTING//"`' -- \
		    $cur ) )
		fi
		;;
	*)
		;;
	esac

} 
[ "$have" ] && complete -F _iptables iptables

# tcpdump(8) completion
#
have tcpdump &&
_tcpdump()
{
	local cur

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}

	COMPREPLY=( $( compgen -W 'host net port src dst ether gateway \
				   less greater' -- $cur ) )

}
[ "$have" ] && complete -F _tcpdump tcpdump

# This meta-cd function observes the CDPATH variable, so that cd additionally
# completes on directories under those specified in CDPATH.
#
_cd()
{
	local IFS=$'\t\n' cur=${COMP_WORDS[COMP_CWORD]} dirs=() i

	_expand || return 0

	# Use standard dir completion if no CDPATH or parameter starts with /,
	# ./ or ../
	if [ -z "$CDPATH" ] || [[ "$cur" == ?(.)?(.)/* ]]; then
		_filedir -d
		return 0
	fi
	IFS=$'\t'
	# we have a CDPATH, so loop on its contents
	for i in ${CDPATH//:/$'\t'}; do
		# create an array of matched subdirs
		dirs=( $( compgen -d $i/$cur ) )
		# add subdirs to list of completions as necessary
		[ ${#dirs[@]} ] && COMPREPLY=( ${COMPREPLY[@]} ${dirs[@]#$i/})
	done
	IFS=$' \t\n'
	_filedir -d
	if [ -n "$CDPATH" ]; then
		for (( i=0; i < ${#COMPREPLY[@]}; i++ )); do
			# remove leading ./ from completions
			COMPREPLY[i]=${COMPREPLY[i]#.\/}
			# turn relative paths from current dir into
			# absolute ones
			[[ ${COMPREPLY[i]} != */* ]] && \
				[ -d "${COMPREPLY[i]}" ] && \
				COMPREPLY[i]=$PWD/${COMPREPLY[i]}
		done
	fi

	return 0
}
complete -F _cd -o filenames cd

# A meta-command completion function for commands like sudo(8), which need to
# first complete on a command, then complete according to that command's own
# completion definition - currently not quite foolproof (e.g. mount and umount
# don't work properly), but still quite useful
#
_command()
{
	local cur func cline cspec

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}

	if [ $COMP_CWORD -eq 1 ]; then
		COMPREPLY=( $( compgen -c -- $cur ) )
	elif complete -p ${COMP_WORDS[1]} &>/dev/null; then
		cspec=$( complete -p ${COMP_WORDS[1]} )
		if [ "${cspec#*-F }" != "$cspec" ]; then
			# complete -F <function>
			#
			# COMP_CWORD and COMP_WORDS() are not read-only,
			# so we can set them before handing off to regular
			# completion routine

			# set current token number to 1 less than now
			COMP_CWORD=$(( $COMP_CWORD - 1 ))
			# get function name
			func=${cspec#*-F }
			func=${func%% *}
			# get current command line minus initial command
			cline="${COMP_LINE#$1 }"
			# split current command line tokens into array
			COMP_WORDS=( $cline )
			$func $cline
			# remove any \: generated by a command that doesn't
			# default to filenames or dirnames (e.g. sudo chown)
			if [ "${cspec#*-o }" != "$cspec" ]; then
				cspec=${cspec#*-o }
				cspec=${cspec%% *}
				if [[ "$cspec" != @(dir|file)names ]]; then
					COMPREPLY=( "${COMPREPLY[@]//\\\\:/:}" )
				fi
			fi
		elif [ "${cspec#*-[abcdefgjkvu]}" != "$cspec" ]; then
			# complete -[abcdefgjkvu]
			func=$( echo $cspec | \
				sed -e 's/^.*\(-[abcdefgjkvu]\).*$/\1/' )
			COMPREPLY=( $( compgen $func -- $cur ) )
		elif [ "${cspec#*-A}" != "$cspec" ]; then
			# complete -A <type>
			func=${cspec#*-A }
			func=${func%% *}
			COMPREPLY=( $( compgen -A $func -- $cur ) )
		fi
	fi

	[ ${#COMPREPLY[@]} -eq 0 ] && _filedir
}
complete -F _command -o filenames nohup exec nice eval strace time

_root_command()
{
	PATH=$PATH:/sbin:/usr/sbin:/usr/local/sbin _command $1
}
complete -F _root_command -o filenames sudo fakeroot

# ant(1) completion
#
have nslookup &&
_nslookup()
{
	local cur

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]#-}

	COMPREPLY=( $( compgen -P '-' -W 'all class= debug d2 domain= \
			       srchlist= defname search port= querytype= \
			       type= recurse retry root timeout vc \
			       ignoretc' -- $cur ) )
}
[ "$have" ] && complete -F _nslookup nslookup

# mysqladmin(1) completion
#
have mysqladmin &&
_mysqladmin()
{
	local cur prev

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]} 
	prev=${COMP_WORDS[COMP_CWORD-1]}

	case "$prev" in
	-u)
		COMPREPLY=( $( compgen -u -- $cur ) )
		return 0
		;;
	*)
		;;
	esac

	COMPREPLY=( $( compgen -W '-# -f -? -C -h -p -P -i -r -E -s -S -t -u \
					      -v -V -w' -- $cur ) )

	COMPREPLY=( ${COMPREPLY[@]} \
		    $( compgen -W 'create drop extended-status flush-hosts \
				   flush-logs flush-status flush-tables \
				   flush-threads flush-privileges kill \
				   password ping processlist reload refresh \
				   shutdown status variables version' \
		       -- $cur ) )
}
[ "$have" ] && complete -F _mysqladmin mysqladmin

# gzip(1) and bzip2(1) completion
#
have gzip &&
_zip()
{
	local IFS cur prev xspec

	IFS=$'\t\n'
	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	[ ${COMP_WORDS[0]} = "gzip" ] && xspec="*.gz"
	[ ${COMP_WORDS[0]} = "bzip2" ] && xspec="*.bz2"
	[[ "$prev" == -*d* ]] && xspec="!"$xspec

	_expand || return 0

	COMPREPLY=( $( compgen -f -X "$xspec" -- $cur ) \
		    $( compgen -d -- $cur ) )
}
[ "$have" ] && complete -F _zip -o filenames gzip bzip2

# openssl(1) completion
#

# screen(1) completion
#
have screen &&
_screen()
{
	local cur prev

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	case "$prev" in
	-[rR])
		# list detached
		COMPREPLY=( $( \screen -ls | sed -ne 's|^['$'\t'']\+\('$cur'[^'$'\t'']\+\).*Detached.*$|\1|p' ) )
		;;
	-[dDx])
		# list attached
		COMPREPLY=( $( \screen -ls | sed -ne 's|^['$'\t'']\+\('$cur'[^'$'\t'']\+\).*Attached.*$|\1|p' ) )
		;;
	-s)
		# shells
		COMPREPLY=( $( grep ^${cur:-[^#]} /etc/shells ) )
		;;
	*)
		;;
	esac

	return 0
}
[ $have ] && complete -F _screen -o default screen

# ncftp(1) bookmark completion
#
have ncftp &&
_ncftp()
{
	local cur

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}

	if [ $COMP_CWORD -eq 1 ] && [ -f ~/.ncftp/bookmarks ]; then
	    COMPREPLY=( ` compgen -W '$( sed -ne "s/^\([^,]\{1,\}\),.*$/\1/p" \
			   ~/.ncftp/bookmarks )' -- $cur ` )
	fi

	return 0
}
[ $have ] && complete -F _ncftp -o default ncftp

# gdb(1) completion
#
have gdb &&
_gdb()
{
	local cur prev

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	if [ $COMP_CWORD -eq 1 ]; then
		COMPREPLY=( $( compgen -c -- $cur ) )
	elif [ $COMP_CWORD -eq 2 ]; then
		# escaoe slashes for awk
		prev=${prev//\//\\\/}
		COMPREPLY=( ${COMPREPLY[@]} $( ps ahx | sed -e 's#[]\[()]##g' |\
					awk '{p=$5;sub("^.*/","",p); \
					if (p ~ /^'$prev'/) print $1}' | \
					sed -e 's#^.*/##' ))
	fi
}
[ $have ] && complete -F _gdb -o default gdb

# psql(1) completion
#
# gcc(1) completion
#
# The only unusual feature is that we don't parse "gcc --help -v" output
# directly, because that would include the options of all the other backend
# tools (linker, assembler, preprocessor, etc) without any indication that
# you cannot feed such options to the gcc driver directly.  (For example, the
# linker takes a -z option, but you must type -Wl,-z for gcc.)  Instead, we
# ask the driver ("g++") for the name of the compiler ("cc1"), and parse the
# --help output of the compiler.
#
have gcc &&
_gcc()
{
	local cur cc backend

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}

	_expand || return 0

	case "$1" in
	gcj)
		backend=jc1
		;;
	gpc)
		backend=gpc1
		;;
	*77)
		backend=f771
		;;
	*)
		backend=cc1	# (near-)universal backend
		;;
	esac

	if [[ "$cur" == -* ]]; then
		cc=$( $1 -print-prog-name=$backend )
		# sink stderr:
		# for C/C++/ObjectiveC it's useless
		# for FORTRAN/Java it's an error
		COMPREPLY=( $( $cc --help 2>/dev/null | tr '\t' ' ' | \
			       sed -e '/^  *-/!d' -e 's/ *-\([^ ]*\).*/-\1/' | \
			       grep ^$cur | sort -u ) )
	else
		_filedir
	fi
}
[ "$have" ] && complete -o filenames -F _gcc gcc g++ c++ g77 gcj gpc
[ $OS = Linux ] && complete -o filenames -F _gcc cc

# Linux cardctl(8) completion
#
have cardctl &&
_cardctl()
{
	local cur
	COMPREPLY=()

	cur=${COMP_WORDS[COMP_CWORD]}

	if [ $COMP_CWORD -eq 1 ]; then
		COMPREPLY=( $( compgen -W 'status config ident suspend \
					   resume reset eject insert scheme' \
			       -- $cur ) )
	fi
}
[ "$have" ] && complete -F _cardctl cardctl

# This function is required by _dpkg() and _dpkg-reconfigure()

# Debian Linux dpkg(8) completion
#
have java &&
_java()
{
	local cur prev classpath i

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	case $prev in
		-@(cp|classpath))
			_filedir
			return 0
			;;
	esac

	if [[ "$cur" == -* ]]; then
		# relevant options completion
		COMPREPLY=( $( compgen -W '-client -hotspot -server -classic \
			       -cp -classpath -D -verbose -version \
			       -showversion -? -help -X' -- $cur ) )
	else
		# available classes completion
		# find wich classpath to use
		if [ -n "$CLASSPATH" ]; then
			classpath=$CLASSPATH
		else
			classpath=.
		fi
		for (( i=1; i < COMP_CWORD; i++ )); do
			if [[ "${COMP_WORDS[i]}" == -classpath ]] || \
			   [[ "${COMP_WORDS[i]}" == -cp ]]; then
				classpath=${COMP_WORDS[i+1]}
				break
			fi
		done
		# parse each classpath element for classes
		for i in ${classpath//:/ }; do
			if [ -f $i ] && [[ "$i" == *.@(jar|zip) ]]; then
				COMPREPLY=( ${COMPREPLY[@]} $( jar tf $i | \
				grep '\.class' | sed -e 's|\.class||g' \
						  -e 's|/|.|g' | grep ^$cur ) )
			elif [ -d $i ]; then
				COMPREPLY=( ${COMPREPLY[@]} $( find $i -type f \
					    -name \*.class | \
					    sed -e 's|^'$i'/||' \
						-e 's|\.class$||' \
						-e 's|/|.|g' | grep ^$cur ) )
			fi
		done
	fi
}
[ "$have" ] && complete -F _java java

_configure_func()
{
	local cur

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}

	[[ "$cur" != -* ]] && return 0

	COMPREPLY=( $( $1 --help | sed -ne 's|^ *\('$cur'[^ '$'\t'',[]\+\).*$|\1|p' ) )
}
complete -F _configure_func -o default configure

_filedir_xspec()
{
	local IFS cur xspec

	IFS=$'\t\n'
	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}

	_expand || return 0

	# get first exclusion compspec that matches this command
	xspec=$( sed -ne '/ '${1##*/}'/{p;q;}' $BASH_COMPLETION )
	# prune to leave nothing but the -X spec
	xspec=${xspec#*-X }
	xspec=${xspec%% *}

	COMPREPLY=( $( eval compgen -f -X "$xspec" -- \"$cur\" ) \
		    $( compgen -d -- $cur ) )
}
list=( $( sed -ne '/^# START exclude/,/^# FINISH exclude/p' \
	  $BASH_COMPLETION | \
	# read exclusion compspecs
	(
	while read line
	do
		# ignore compspecs that are commented out
		if [ "${line#\#}" != "$line" ]; then continue; fi
		line=${line%# START exclude*}
		line=${line%# FINISH exclude*}
		line=${line##*\'}
		list=( ${list[@]} $line )
	done
	echo ${list[@]}
	)
     ) )
# remove previous compspecs
if [ ${#list[@]} -gt 0 ]; then
    eval complete -r ${list[@]}
    # install new compspecs
    eval complete -F _filedir_xspec -o filenames ${list[@]}
fi
unset list[@]

# source completion directory definitions
if [ -d $BASH_COMPLETION_DIR -a -r $BASH_COMPLETION_DIR -a \
     -x $BASH_COMPLETION_DIR ]; then
	for i in $BASH_COMPLETION_DIR/*; do
		[ -r $i ] && . $i
	done
fi
# source user completion file
[ $BASH_COMPLETION != ~/.bash_completion -a -r ~/.bash_completion ] \
	&& . ~/.bash_completion
unset -f have
unset OS RELEASE have i

