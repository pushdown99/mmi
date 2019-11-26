#!/bin/bash
core=$(grep -c 'processor' /proc/cpuinfo)

if [ $core -eq 0 ]; then
        core=1
fi

cat /proc/loadavg \
| awk 'BEGIN {printf "\n%-20s%-20s\n--------------------------------------------\n", "MINUTE", "CPU LOAD"} \
    { printf "%-20s%-20s\n%-20s%-20s\n%-20s%-20s\n", "1 min", ($1*100)/'$core', "5 min", ($2*100)/'$core', "15 min", ($3*100)/'$core' } \
    END {print ""}' \
| /bin/sed 'N;$s/,\n/\n/;P;D'
