#!/bin/bash
ps axo pid,user,pmem,rss,vsz,comm --sort -pmem,-rss,-vsz | head -n 15 \
| awk 'BEGIN {printf "\n%-10s%-15s%-10s%-10s%-10s%-15s \n--------------------------------------------------------------------\n",  \
    "PID", "USER", "MEM%", "RSS", "VSZ", "CMD"} \
    NR>1 {printf "%-10s%-15s%-10s%-10s%-10s%-15s \n", $1, $2, $3, $4, $5, $6} \
    END {print ""}' \
| /bin/sed 'N;$s/,\n/\n/;P;D'
