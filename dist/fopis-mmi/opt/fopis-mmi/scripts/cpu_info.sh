#!/bin/bash
echo "";
/usr/bin/lscpu \
| /bin/sed 'N;$s/,\n/\n/;P;D'
echo "";
