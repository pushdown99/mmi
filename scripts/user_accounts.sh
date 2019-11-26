#!/bin/bash

cat /etc/passwd \
| awk -F: 'BEGIN {printf "\n\n%-10s%-25s%-15s \n----------------------------------------------------------\n",  \
        "TYPE", "USER", "HOME"}
  { if ($3<=499){userType="system";} 
    else {userType="user";} \
    printf "%-10s%-25s%-15s \n", userType, $1, $6 } \
  END {print ""}' \
| /bin/sed 'N;$s/,\n/\n/;P;D'
