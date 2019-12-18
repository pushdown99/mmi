#include <stdio.h>

#include "mmc.h"

extern void quit(const char* va_alist,...);
extern void startS(const char* va_alist,...);
extern void stopS(const char* va_alist,...);
extern void args_ip(const char* va_alist,...);
extern void args_string(const char* va_alist,...);
extern void args_numeric(const char* va_alist,...);
extern void args_command(const char* va_alist,...);

extern void show_arp_cache(const char* va_alist,...);
extern void show_ps(const char* va_alist,...);
extern void show_radix_checkup(const char* va_alist,...);
extern void show_radix_config(const char* va_alist,...);
extern void show_bandwidth(const char* va_alist,...);
extern void show_application(const char* va_alist,...);
extern void show_cpu_info(const char* va_alist,...);
extern void show_cpu_process(const char* va_alist,...);
extern void show_cpu_temp(const char* va_alist,...);
extern void show_cpu_utilization(const char* va_alist,...);
extern void show_cron_history(const char* va_alist,...);
extern void show_current_ram(const char* va_alist,...);
extern void show_disk_partitions(const char* va_alist,...);
extern void show_docker_processes(const char* va_alist,...);
extern void show_download_rate(const char* va_alist,...);
extern void show_general_info(const char* va_alist,...);
extern void show_internet_speed(const char* va_alist,...);
extern void show_io_stats(const char* va_alist,...);
extern void show_ip_addresses(const char* va_alist,...);
extern void show_cpu_load(const char* va_alist,...);
extern void show_login_users(const char* va_alist,...);
extern void show_memcached(const char* va_alist,...);
extern void show_memory_info(const char* va_alist,...);
extern void show_network_connections(const char* va_alist,...);
extern void show_cpu_number(const char* va_alist,...);
extern void show_ping(const char* va_alist,...);
extern void show_pm2(const char* va_alist,...);
extern void show_raid_status(const char* va_alist,...);
extern void show_ram_process(const char* va_alist,...);
extern void show_recent_logins(const char* va_alist,...);
extern void show_redis(const char* va_alist,...);
extern void show_scheduled_crons(const char* va_alist,...);
extern void show_swap(const char* va_alist,...);
extern void show_upload_rate(const char* va_alist,...);
extern void show_user_accounts(const char* va_alist,...);

mmc_t   _mmc[]={
    {"hist",        0,1,'C',    "hist",         "Histoty",                   	NULL},
    {"quit",        0,0,'C',    "quit",         "Quit this Program",         	quit},
    {"help",        0,0,'C',    "help",         "This screen",               	mmchelp}, /* internal support function */
    {"start",       0,1,'C',    "start",        "start function",            	NULL},
    {"stop",        0,1,'C',    "stop",        	"stop function",             	NULL},
    {"show",        0,1,'C',    "show",        	"show function",             	NULL},
    {"args",        0,1,'C',    "args",        	"argument function",         	NULL},

    {"{arg}",       1,0,'C',    "start-S",          "",         				startS},
    {"{arg}",       1,0,'C',    "stop-S",          "",         				        stopS},
    {"arp",         1,1,'C',    "show-arp",         "",         				NULL},
    {"ps",          1,1,'C',    "show-ps",         "",         				        NULL},
    {"radix",       1,1,'C',    "show-radix",       "",         				NULL},
    {"bandwidth",   1,0,'C',    "show-bandwidth",   "",         				show_bandwidth},
    {"application", 1,0,'C',    "show-application", "",         				show_application},
    {"ping",        1,0,'C',    "show-ping",        "",         				show_ping},
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
    {"{process}",   2,0,'S',    "show-ps-S",               "",         				show_ps},
    {"checkup",     2,0,'C',    "show-radix-checkup",      "",         				show_radix_checkup},
    {"config",      2,0,'C',    "show-radix-config",       "",         				show_radix_config},
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
