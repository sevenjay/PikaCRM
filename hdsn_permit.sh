#!/bin/bash
#
# 

# Find a program on PATH
_foundprog=
findprog () {
    _foundprog=`which $1 2>/dev/null`
    return $?
}

# absolutize dir
dirname=`dirname $0`
cd $dirname
#echo `pwd`

file=hdsn

#make sure hdsn is in group disk
_group=`ls -g $file|cut -d ' ' -f3`

#make sure hdsn has set-group-id bit
[ -g $file ] && Gbit="Yes" || Gbit="No"


# See if we have gksudo or kdesudo
_sudo=
if findprog gksudo; then
    _sudo=gksudo
elif findprog kdesudo; then
    _sudo=kdesudo
else
    _sudo=sudo
fi

#echo $file
#echo $Gbit
#echo $_group
#echo $_sudo

if [ "$_group" != "disk" ] ; then
    `$_sudo chgrp disk $file` || fail=1
fi

if [ "$Gbit" != "Yes" ] ; then
    `$_sudo chmod g+s $file` || fail=1 
fi

#if [ fail != 1 ] ; then
#$dirname/PikaCRM
#fi
