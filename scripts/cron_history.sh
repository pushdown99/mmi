#!/bin/bash
# Month, Day, Time, Hostname, tag, user,

grep -m50 CRON /var/log/syslog \
| awk 'BEGIN {printf "\n\n%-16s%-10s%-s\n-----------------------------------------------------------------------------------------------------\n", \
    "TIME","USER","MESSAGE","",""} \
    { s = ""; for (i = 6; i <= NF; i++) s = s $i " "; \
    s = $5 " " s; \
    printf "%-4s%-3s%-9s%-10s%-s\n", $1, $2, $3, $6, s} \
    END {print ""}' \
| /bin/sed 'N;$s/,\n/\n/;P;D'

