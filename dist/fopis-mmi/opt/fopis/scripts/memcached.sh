#!/bin/bash
echo "stats" | /bin/nc -w 1 127.0.0.1 11211 | /bin/grep 'bytes' \
| awk 'BEGIN {printf "\n\n%-25s%-15s\n--------------------------------------\n", "PARAMETER", "VALUES"} \
    {printf "%-25s%-15s\n", $2, $3 } END {print ""}' \
| tr '\r' ' ' \
| sed 'N;$s/,\n/\n/;P;D'

