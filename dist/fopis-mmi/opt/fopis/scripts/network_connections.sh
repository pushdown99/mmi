#!/bin/bash
netstat -ntu | awk 'NR>2 {print $5}' | sort | uniq -c \
| awk 'BEGIN {printf "\n\n%-15s%-25s \n----------------------------------------------------------\n",  \
  "CONNECTIONS", "FOREIGN ADDRESS"} 
  {printf "%-15s%-25s \n", $1, $2} \
  END {print ""}' \
| /bin/sed 'N;$s/,\n/\n/;P;D'
