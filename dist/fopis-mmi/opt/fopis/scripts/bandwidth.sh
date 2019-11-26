#!/bin/bash
/bin/cat /proc/net/dev \
| awk 'BEGIN {printf "\n\n%-25s%-15s%-15s \n----------------------------------------------------------\n",  \
	"INTERFACE", "TX", "RX"} NR>2 { printf "%-25s%-15s%-15s \n", $1, $2, $10 } \
        END {print ""}' \
| /bin/sed 'N;$s/,\n/\n/;P;D'
