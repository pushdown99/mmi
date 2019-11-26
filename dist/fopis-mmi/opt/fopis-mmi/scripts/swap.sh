#!/bin/bash
cat /proc/swaps \
| awk 'BEGIN {printf "\n\n%-20s%-15s%-15s%-15s%-15s \n-------------------------------------------------------------------------\n",  \
      "FILENAME", "TYPE", "SIZE", "USED", "PRIORITY"} \
    NR>1 {printf "%-20s%-15s%-15s%-15s%-15s \n", $1, $2, $3, $4, $5} \
    END {print ""}' \
| /bin/sed 'N;$s/,\n/\n/;P;D'

