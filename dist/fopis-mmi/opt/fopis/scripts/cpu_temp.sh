#!/bin/bash
sensors \
| awk 'BEGIN {printf "\n\nCPU TEMPERATURES\n---------------------------------------------------------\n"} \
  NR>2 { print $0 } \
  END {print ""}'
