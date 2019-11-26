#!/bin/bash
containers="$(docker ps | awk '{if(NR>1) print $NF}')"
printf "\n\n%-35s%-10s%-10s%-10s%-10s%-10s \n--------------------------------------------------------------------------------\n" "CNAME" "PID" "USER" "CPU%" "MEM%" "CMD"

for i in $containers; do
  docker top $i axo pid,user,pcpu,pmem,comm --sort -pcpu,-pmem | head -n 15 \
  | awk -v cnt="$i" ' \
    NR>2 { printf "%-35s%-10s%-10s%-10s%-10s%-10s \n", cnt, $1, $2, $3, $4, $5 }' \
  | /bin/sed 'N;$s/,\n/\n/;P;D'
done

echo ""
