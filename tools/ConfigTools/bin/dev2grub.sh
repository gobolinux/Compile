#!/bin/sh

#
# Maps the usual /dev hard disk names to GRUB's format
# Written by Lucas Correia Villa Real <lucasvr@gobolinux.org>
#

if [ $# != 1 ]
then
    echo "Syntax: $0 <disk>"
    echo "where disk=hda1, sda2, etc"
    exit 1
fi

if [ ! -b /dev/$1 ]
then
    echo "/dev/$1 is not a valid partition!"
    exit 1
fi

target_disk=$(echo $1 | sed 's/[0-9]//g')
target_partition=$(echo $1 | sed 's/[a-zA-Z]//g')

disk_prefix=$(echo $target_disk | cut -b1-2)
disk_count=-1
partition_count=-1

# find grub's disk
for i in /dev/${disk_prefix}?
do
    if ls ${i}[0-9] 2> /dev/null > /dev/null
    then
        disk_count=$(( disk_count + 1 ))
        [ "$i" = "/dev/$target_disk" ] && break
    fi
done

# find grub's partition
for i in /dev/${target_disk}*
do
    [ "$i" = "/dev/$1" ] && break
    partition_count=$(( partition_count + 1 ))
done

if [ $partition_count = -1 ]
then
    echo "(hd$disk_count)"
else
    echo "(hd$disk_count,$partition_count)"
fi
