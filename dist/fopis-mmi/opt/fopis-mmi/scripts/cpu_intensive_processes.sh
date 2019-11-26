#!/bin/bash

/bin/ps axo pid,user,pcpu,rss,vsz,comm --sort -pcpu,-rss,-vsz | head -n 15 \
| awk 'BEGIN {printf "\n\n%-10s%-10s%-10s%-10s%-10s%-10s \n----------------------------------------------------------\n",  \
    "PID", "USER", "CPU%", "RSS", "VSZ", "CMD"} \
    NR>1 { printf "%-10s%-10s%-10s%-10s%-10s%-10s \n", $1, $2, $3, $4, $5, $6 } \
    END {print ""}' \
| /bin/sed 'N;$s/,\n/\n/;P;D'
