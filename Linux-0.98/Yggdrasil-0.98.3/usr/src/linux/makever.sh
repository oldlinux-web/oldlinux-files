#! /bin/sh

if [ ! -f .version ]
then
	echo 0 > .version
fi
cycle=`cat .version`
cycle=`expr $cycle + 1`
if [ $cycle -gt 99 ]
then
	cycle=0
fi
echo $cycle > .version
