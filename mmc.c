#include <stdio.h>

#include "mmc.h"

extern void quit(char*,...);
extern void start(char*,...);
extern void stop(char*,...);
extern void args_ip(char*,...);
extern void args_string(char*,...);
extern void args_numeric(char*,...);
extern void args_command(char*,...);

extern void show_arp_cache(char* fmt,...);
extern void show_bandwidth(char* fmt,...);
extern void show_application(char* fmt,...);
extern void show_cpu_info(char* fmt,...);
extern void show_cpu_process(char* fmt,...);
extern void show_cpu_temp(char* fmt,...);
extern void show_cpu_utilization(char* fmt,...);
extern void show_cron_history(char* fmt,...);
extern void show_current_ram(char* fmt,...);
extern void show_disk_partitions(char* fmt,...);
extern void show_docker_processes(char* fmt,...);
extern void show_download_rate(char* fmt,...);
extern void show_general_info(char* fmt,...);
extern void show_internet_speed(char* fmt,...);
extern void show_io_stats(char* fmt,...);
extern void show_ip_addresses(char* fmt,...);
extern void show_cpu_load(char* fmt,...);
extern void show_login_users(char* fmt,...);
extern void show_memcached(char* fmt,...);
extern void show_memory_info(char* fmt,...);
extern void show_network_connections(char* fmt,...);
extern void show_cpu_number(char* fmt,...);
extern void show_ping(char* fmt,...);
extern void show_pm2(char* fmt,...);
extern void show_raid_status(char* fmt,...);
extern void show_ram_process(char* fmt,...);
extern void show_recent_logins(char* fmt,...);
extern void show_redis(char* fmt,...);
extern void show_scheduled_crons(char* fmt,...);
extern void show_swap(char* fmt,...);
extern void show_upload_rate(char* fmt,...);
extern void show_user_accounts(char* fmt,...);

mmc_t   _mmc[]={
    {"hist",        0,1,'C',    "hist",         "Histoty",                   	NULL},
    {"quit",        0,0,'C',    "quit",         "Quit this Program",         	quit},
    {"help",        0,0,'C',    "help",         "This screen",               	mmchelp}, /* internal support function */
    {"start",       0,0,'C',    "start",        "start function",            	start},
    {"stop",        0,0,'C',    "stop",        	"stop function",             	stop},
    {"show",        0,1,'C',    "show",        	"show function",             	NULL},
    {"args",        0,1,'C',    "args",        	"argument function",         	NULL},

    {"arp",         1,1,'C',    "show-arp",         "",         				NULL},
    {"bandwidth",   1,0,'C',    "show-bandwidth",   "",         				show_bandwidth},
    {"application", 1,0,'C',    "show-application", "",         				show_application},
    {"cpu",         1,1,'C',    "show-cpu",         "",         				NULL},
    {"cron",        1,1,'C',    "show-cron",        "",         				NULL},
    {"memory",      1,1,'C',    "show-memory",      "",         				NULL},
    {"disk",        1,1,'C',    "show-disk",        "",         				NULL},
    {"docker",      1,1,'C',    "show-docker",      "",         				NULL},
    {"rate",        1,1,'C',    "show-rate",        "",         				NULL},

    {"ip",          1,1,'C',    "args-ip",          "",         				NULL},
    {"string",      1,1,'C',    "args-string",      "",         				NULL},
    {"numeric",     1,1,'C',    "args-numeric",     "",         				NULL},
    {"command",     1,1,'C',    "args-command",     "",         				NULL},

    {"cache",       2,0,'C',    "show-arp-cache",          "",         				show_arp_cache},
    {"info",        2,0,'C',    "show-cpu-info",           "",         				show_cpu_info},
    {"process",     2,0,'C',    "show-cpu-process",        "",         				show_cpu_process},
    {"temp",        2,0,'C',    "show-cpu-temp",           "",         				show_cpu_temp},
    {"utilization", 2,0,'C',    "show-cpu-utilization",    "",         				show_cpu_utilization},
    {"load",        2,0,'C',    "show-cpu-load",           "",         				show_cpu_load},
    {"history",     2,0,'C',    "show-cron-history",       "",         				show_cron_history},
    {"capacity",    2,0,'C',    "show-memory-capacity",    "",         				show_current_ram},
    {"info",        2,0,'C',    "show-memory-info",        "",         				show_memory_info},
    {"partitions",  2,0,'C',    "show-disk-partitions",    "",         				show_disk_partitions},
    {"processes",   2,0,'C',    "show-docker-processes",   "",         				show_docker_processes},
    {"download",    2,0,'C',    "show-rate-download",      "",         				show_download_rate},
    {"upload",      2,0,'C',    "show-rate-upload",        "",         				show_upload_rate},

    {"{arg}",       2,0,'I',    "args-ip-I",               "",         				args_ip},
    {"{arg}",       2,0,'S',    "args-string-S",           "",         				args_string},
    {"{arg}",       2,0,'N',    "args-numeric-N",          "",         				args_numeric},
    {"arg",         2,0,'C',    "args-command-arg",        "",         				args_command},

    {"",-1,0,0,"","",NULL}      /* <-- Remain this - critical issue (EOF) */
};
