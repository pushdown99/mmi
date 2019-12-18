#include <stdio.h>
#include <stdlib.h>
#include <json.h>
#include <string.h>
#include <stdarg.h>

#include "mmc.h"

char trademark[]={
    "--------------------------------------------------\n"
    "RADIX IoT Platform MMI (Man-Machine Interface)    \n"
    "Copyright(C) 2019 Ariatech                        \n"
    "                                                  \n"
    "                       http://ariatech.kr         \n"
    "--------------------------------------------------\n"
};

extern mmc_t _mmc[];

int
main()
{
	char buf[BUFSIZ],mml[BUFSIZ],*p;
	int prev,line,cont;

	regmmc(_mmc);

    /* Initialize */
    system("clear");
    printf("%s",trademark);

    /* MML */
    bzero(buf,BUFSIZ);
    printf("\rPress the <tab> key at any time for completions.\n\n");

    for(line=0,cont=1;;cont=1) {
        /* Command Line (Prompt) */
        printf("MMC:%d # %s",line,buf);
        mmcgets(buf);
        if((p=strchr(buf,'\n'))!=NULL) { *p=0; cont=0;  }
        if((p=strchr(buf,'\t'))!=NULL) { *p=' ';        }
        if(strlen(buf)<=0) continue;

        bzero(mml,BUFSIZ);
        strcpy(mml,buf); prev=line;
        line=pparse(buf,cont);

		if(!cont) bzero(buf,BUFSIZ);
	}
	return 1;
}

char _internal_buffer[BUFSIZ]  = "";
char _internal_command[BUFSIZ] = "";
char _internal_path[BUFSIZ]    = "";
json_object* _internal_object;

char* getOutput(char *script)
{
  FILE *stream = NULL;
  char _buf[BUFSIZ];
  _internal_buffer[0] = 0;

  //char *home = getenv("FOPIS_MMI_PATH");
  char *home = "/opt/radix";
  sprintf(_internal_command, "%s/scripts/%s 2>&1", home, script);
//printf("%s \n", _internal_command);

//printf("COMMAND: %s \n\n", _internal_command);
  stream = popen(_internal_command, "r");
  if (stream) {
    while (!feof(stream))
      if (fgets(_buf, BUFSIZ, stream) != NULL) strcat(_internal_buffer, _buf);
    pclose(stream);
  }
//printf("OUTPUT: %s \n\n", _internal_buffer);
  printf("%s \n", _internal_buffer);
  return _internal_buffer;
}

json_object* getJson(char *cmd)
{
  char *buffer = getOutput(cmd);

  _internal_object = json_tokener_parse(buffer);
  //printf("(JSON Object)\n\n%s\n\n", json_object_to_json_string_ext(_internal_object, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));
  //printf("\n%s\n\n", json_object_to_json_string_ext(_internal_object, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));
  return _internal_object;
}

#define ARP_CACHE_SCRIPT           "arp_cache.sh"
#define BANDWIDTH_SCRIPT           "bandwidth.sh"
#define APPLICATION_SCRIPT         "common_applications.sh"
#define CPU_INFO_SCRIPT            "cpu_info.sh"
#define CPU_PROCESS_SCRIPT         "cpu_intensive_processes.sh"
#define CPU_TEMP_SCRIPT            "cpu_temp.sh"
#define CPU_UTILIZATION_SCRIPT     "cpu_utilization.sh"
#define CRON_HISTORY_SCRIPT        "cron_history.sh"
#define CURRENT_RAM_SCRIPT         "current_ram.sh"
#define DISK_PARTITIONS_SCRIPT     "disk_partitions.sh"
#define DOCKER_PROCESSES_SCRIPT    "docker_processes.sh"
#define DOWNLOAD_RATE_SCRIPT       "download_transfer_rate.sh"
#define GENERAL_INFO_SCRIPT        "general_info.sh"
#define INTERNET_SPEED_SCRIPT      "internet_speed.sh"
#define IO_STATS_SCRIPT            "io_stats.sh"
#define IP_ADDRESSES_SCRIPT        "ip_addresses.sh"
#define CPU_LOAD_SCRIPT            "load_avg.sh"
#define LOGGED_IN_USERS_SCRIPT     "logged_in_users.sh"
#define MEMCACHED_SCRIPT           "memcached.sh"
#define MEMORY_INFO_SCRIPT         "memory_info.sh"
#define NETWORK_CONNECTIONS_SCRIPT "network_connections.sh"
#define NUMBER_OF_CPU_SCRIPT       "number_of_cpu_cores.sh"
#define PING_SCRIPT                "ping.sh"
#define PM2_SCRIPT                 "pm2.sh"
#define RAID_STATUS_SCRIPT         "raid_status.sh"
#define RAM_PROCESS_SCRIPT         "ram_intensive_processes.sh"
#define RECENT_LOGINS_SCRIPT       "recent_account_logins.sh"
#define REDIS_SCRIPT               "redis.sh"
#define SCHEDULED_CRONS_SCRIPT     "scheduled_crons.sh"
#define SWAP_SCRIPT                "swap.sh"
#define UPLOAD_RATE_SCRIPT         "upload_transfer_rate.sh"
#define USER_ACCOUNTS_SCRIPT       "user_accounts.sh"

void startS(const char* va_alist,...)
{
    DECLARE_VAR;
    GET_VAR_ARG(va_alist);

    printf("args: %s \n", arg2);
}

void stopS(const char* va_alist,...)
{
    DECLARE_VAR;
    GET_VAR_ARG(va_alist);

    printf("args: %s \n", arg2);
}

void show_public_ip(const char* va_alist,...)
{
    DECLARE_VAR;
    GET_VAR_ARG(va_alist);

    printf("address: %s \n", arg2);
}

void show_radix_checkup(const char* va_alist,...)
{
    DECLARE_VAR;
    GET_VAR_ARG(va_alist);

    system("/opt/radix/bin/radix-checkup");
}

void show_radix_config(const char* va_alist,...)
{
    DECLARE_VAR;
    GET_VAR_ARG(va_alist);

    system("/opt/radix/bin/radix-config");
}

void show_arp_cache(const char* va_alist,...)
{
  char* out = getJson(ARP_CACHE_SCRIPT);
}

void show_ps(const char* va_alist,...)
{
    DECLARE_VAR;
    GET_VAR_ARG(va_alist);

    char cmd[BUFSIZ];
    sprintf(cmd, "ps -ef | grep %s | grep -v grep", arg3);
    system(cmd);
}

void show_bandwidth(const char* va_alist,...)
{
  char* out = getJson(BANDWIDTH_SCRIPT);
}

void show_application(const char* va_alist,...)
{
  char* out = getJson(APPLICATION_SCRIPT);
}

void show_cpu_info(const char* va_alist,...)
{
  char* out = getJson(CPU_INFO_SCRIPT);
}

void show_cpu_process(const char* va_alist,...)
{
  char* out = getJson(CPU_PROCESS_SCRIPT);
}

void show_cpu_temp(const char* va_alist,...)
{
  char* out = getJson(CPU_TEMP_SCRIPT);
}

void show_cpu_utilization(const char* va_alist,...)
{
  char* out = getJson(CPU_UTILIZATION_SCRIPT);
}

void show_cron_history(const char* va_alist,...)
{
  char* out = getJson(CRON_HISTORY_SCRIPT);
}

void show_current_ram(const char* va_alist,...)
{
  char* out = getJson(CURRENT_RAM_SCRIPT);
}

void show_disk_partitions(const char* va_alist,...)
{
  char* out = getJson(DISK_PARTITIONS_SCRIPT);
}

void show_docker_processes(const char* va_alist,...)
{
  char* out = getJson(DOCKER_PROCESSES_SCRIPT);
}

void show_download_rate(const char* va_alist,...)
{
  char* out = getJson(DOWNLOAD_RATE_SCRIPT);
}

void show_general_info(const char* va_alist,...)
{
  char* out = getJson(GENERAL_INFO_SCRIPT);
}

void show_internet_speed(const char* va_alist,...)
{
  char* out = getJson(INTERNET_SPEED_SCRIPT);
}

void show_io_stats(const char* va_alist,...)
{
  char* out = getJson(IO_STATS_SCRIPT);
}

void show_ip_addresses(const char* va_alist,...)
{
  char* out = getJson(IP_ADDRESSES_SCRIPT);
}

void show_cpu_load(const char* va_alist,...)
{
  char* out = getJson(CPU_LOAD_SCRIPT);
}

void show_login_users(const char* va_alist,...)
{
  char* out = getJson(LOGGED_IN_USERS_SCRIPT);
}

void show_memcached(const char* va_alist,...)
{
  char* out = getJson(MEMCACHED_SCRIPT);
}

void show_memory_info(const char* va_alist,...)
{
  char* out = getJson(MEMORY_INFO_SCRIPT);
}

void show_network_connections(const char* va_alist,...)
{
  char* out = getJson(NETWORK_CONNECTIONS_SCRIPT);
}

void show_cpu_number(const char* va_alist,...)
{
  char* out = getJson(NUMBER_OF_CPU_SCRIPT);
}

void show_ping(const char* va_alist,...)
{
  char* out = getJson(PING_SCRIPT);
}

void show_pm2(const char* va_alist,...)
{
  char* out = getJson(PM2_SCRIPT);
}

void show_raid_status(const char* va_alist,...)
{
  char* out = getJson(RAID_STATUS_SCRIPT);
}

void show_ram_process(const char* va_alist,...)
{
  char* out = getJson(RAM_PROCESS_SCRIPT);
}

void show_recent_logins(const char* va_alist,...)
{
  char* out = getJson(RECENT_LOGINS_SCRIPT);
}

void show_redis(const char* va_alist,...)
{
  char* out = getJson(REDIS_SCRIPT);
}

void show_scheduled_crons(const char* va_alist,...)
{
  char* out = getJson(SCHEDULED_CRONS_SCRIPT);
}

void show_swap(const char* va_alist,...)
{
  char* out = getJson(SWAP_SCRIPT);
}

void show_upload_rate(const char* va_alist,...)
{
  char* out = getJson(UPLOAD_RATE_SCRIPT);
}

void show_user_accounts(const char* va_alist,...)
{
  char* out = getJson(USER_ACCOUNTS_SCRIPT);
}


