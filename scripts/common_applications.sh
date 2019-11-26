#!/bin/bash
whereis php node mysql mongo vim python ruby java apache2 nginx openssl vsftpd make \
| awk 'BEGIN {printf "\n\n%-15s%-35s%-25s \n----------------------------------------------------------\n",  \
  "BINARY", "LOCATION", "INSTALLED"} \
  {if(length($2)==0) { installed="false"; } else { installed="true"; } \
  printf "%-15s%-35s%-25s \n", $1, $2, "installed" } \
  END {print ""}' \
| /bin/sed 'N;$s/,\n/\n/;P;D'
