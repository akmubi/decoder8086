#!/bin/sh

diff -q $1 $2 1> /dev/null 2>&1

if [ $? -eq 0 ]; then
	echo ' OK'
else
	echo ' Failed'
fi
