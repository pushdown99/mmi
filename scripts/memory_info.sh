#!/bin/bash
/bin/cat /proc/meminfo \
| awk 'BEGIN {print "\n"} 
  {printf "%-20s: %-10s\n", $1, $2} 
  END {print ""}' \
| /bin/sed 'N;$s/,\n/\n/;P;D'
