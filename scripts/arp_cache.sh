#!/bin/bash

arp | awk 'BEGIN {printf "\n\n%-20s%-10s%-20s%-8s%-25s \n---------------------------------------------------------------\n", \
	"ADDRESS", "H/W TYPE", "H/W ADDRESS", "FLAGS", "MASK"} \
	NR>1 { printf "%-20s%-10s%-20s%-8s%-25s \n", $1, $2, $3, $4, $5 } \
	END {print ""}' \
| /bin/sed 'N;$s/},/}/;P;D'
