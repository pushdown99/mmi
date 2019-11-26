#!/bin/bash

awkCmd=`which awk`
catCmd=`which cat`
grepCmd=`which grep`
memInfoFile="/proc/meminfo"

# References:
#   Calculations: http://zcentric.com/2012/05/29/mapping-procmeminfo-to-output-of-free-command/
#   Fields: https://www.kernel.org/doc/Documentation/filesystems/proc.txt

cat /proc/meminfo | grep 'MemTotal\|MemFree\|Buffers\|Cached' | paste -sd' ' \
| awk 'BEGIN {printf "\n\n%-15s%-15s%-15s\n---------------------------------------------\n", \
    "TOTAL(mb)", "USED(mb)", "FREE(mb)" } \
    {printf "%-15s%-15s%-15s\n", ($2/1024), ( ($2-($5+$8+$11))/1024 ), (($5+$8+$11)/1024) } \
        END {print ""}' \
| /bin/sed 'N;$s/,\n/\n/;P;D'
