#!/bin/bash
/usr/bin/w -h \
| awk 'BEGIN {printf "\n\n%-15s%-25s%-15s \n----------------------------------------------------------\n",  \
  "USER", "FROM", "WHEN"} \
  { printf "%-15s%-25s%-15s \n", $1, $3, $4 } \
  END {print ""}' \
| /bin/sed 'N;$s/,\n/\n/;P;D'
