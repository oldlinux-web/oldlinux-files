#!/bin/sh
# This script belongs in a public bin directory which should
# be on peoples path. It is used by xdtm.
#
# Execute a program, but don't terminate script until a 
# return.
# For use within an xterm to stop output being missed.
#

echo "Running" $1 "in this window"
$*
echo "======= Press <Return> to close the window ======="
read test

