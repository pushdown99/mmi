#!/bin/bash
# http://askubuntu.com/questions/413367/ping-multiple-ips-using-bash

printf "\n%-25s%-15s \n------------------------------------------------\n" "HOSTNAME" "DELAY(ms)"

cat /opt/radix/scripts/ping_hosts | while read out
do
  delay=$(ping -qc 2 $out | awk -F/ '/^rtt/{print $5}' | /bin/sed 'N;$s/,\n/\n/;P;D')
  printf "%-25s%-15s \n" $out $delay
done
echo ""
