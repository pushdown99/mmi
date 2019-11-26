#!/bin/bash
lastlog -t 365 \
| awk 'BEGIN {printf "\n\n%-10s%-20s%-20s\n------------------------------------------------------------\n", "USER", "IP", "DATE"}
    NR>1 {printf "%-10s%-20s%-s %-s %-s %-s %-s\n", $1, $3, $5, $6, $7, $8, $9}
    END {print ""}' \
| /bin/sed 'N;$s/,\n/\n/;P;D'
