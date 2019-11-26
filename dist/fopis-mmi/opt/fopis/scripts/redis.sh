#!/bin/bash
redis-cli INFO  | grep 'redis_version\|connected_clients\|connected_slaves\|used_memory_human\|total_connections_received\|total_commands_processed' \
| awk -F: 'BEGIN {print ""} \
    {print $1, "", $2 } \
    END {print ""}' \
| /bin/sed 'N;$s/,\n/\n/;P;D'
