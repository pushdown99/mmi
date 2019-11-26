#!/bin/bash

SCRIPTPATH=`dirname $(readlink -f $0)`
SPEED_TEST_SCRIPT=$SCRIPTPATH"/speedtest_cli.py"

$SPEED_TEST_SCRIPT | grep 'Upload\|Download' \
| awk 'BEGIN {printf "\n\n%-15s%-15s \n----------------------------------------------------------\n",  \
  "DIRECTION", "SPEED"} \
  {printf "%-15s%-15s \n", $1, $2} \
  END {print ""}' \
| /bin/sed 'N;$s/",/"/;P;D'
