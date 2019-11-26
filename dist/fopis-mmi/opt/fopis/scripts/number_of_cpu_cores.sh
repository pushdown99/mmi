#!/bin/bash

grep -c 'model name' /proc/cpuinfo \
| awk 'BEGIN {printf "\nNUMBER OF CPU CORES\n-----------------------\n"} \
  {print $1} \
  END {print ""}' \
| /bin/sed 'N;$s/,\n/\n/;P;D'
