#!/bin/bash
/bin/cat /proc/diskstats \
| awk 'BEGIN {printf "\n\n%-15s%-8s%-8s%-20s%-20s \n----------------------------------------------------------\n", "DEVICE", "READS", "WRITES", "IN PROGRESS", "TIME IN I/O"} \
        {if($4==0 && $8==0 && $12==0 && $13==0) next} \
	{printf "%-15s%-8s%-8s%-20s%-20s \n", $3, $4, $8, $12, $13} \
        END {print ""}' \
| /bin/sed 'N;$s/,\n/\n/;P;D'
