#!/bin/bash
/bin/df -Ph | awk  'BEGIN {printf "\n\n%-20s%-10s%-10s%-10s%-10s%-10s \n--------------------------------------------------------------------\n",  \
  "FILESYSTEM", "SIZE", "USED", "AVAIL", "USED", "MOUNTED" } \
  NR>2 { printf "%-20s%-10s%-10s%-10s%-10s%-10s \n", $1, $2, $3, $4, $5, $6 } \
  END {print ""}' \
| /bin/sed 'N;$s/,\n/\n/;P;D'
