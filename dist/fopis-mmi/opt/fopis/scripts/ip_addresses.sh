#!/bin/bash
externalIp=`dig +short myip.opendns.com @resolver1.opendns.com`

printf "\n\n%-20s%-20s\n-------------------------------------------------------------\n" "INTERFACE" "IP ADDRESS"

for item in $(ifconfig | grep -oP "^[a-zA-Z0-9:]*(?=:)"); do 
  ifconfig $item | grep "inet" | awk '{match($0,"inet (addr:)?([0-9.]*)",a)}END{ if (NR != 0){print a[2]; exit}{print "none"}}'
done

printf "%-20s%-20s\n" "external" $externalIp
printf "\n"
