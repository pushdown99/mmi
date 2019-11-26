#include <stdio.h>

#include "mmc.h"

extern void quit(char*,...);
extern void start(char*,...);
extern void stop(char*,...);
extern void args_ip(char*,...);
extern void args_string(char*,...);
extern void args_numeric(char*,...);
extern void args_command(char*,...);

mmc_t   _mmc[]={
    {"hist",        0,1,'C',    "hist",         "Histoty",                   	NULL},
    {"quit",        0,0,'C',    "quit",         "Quit this Program",         	quit},
    {"help",        0,0,'C',    "help",         "This screen",               	mmchelp}, /* internal support function */
    {"start",       0,0,'C',    "start",        "start function",            	start},
    {"stop",        0,0,'C',    "stop",        	"stop function",             	stop},
    {"args",        0,1,'C',    "args",        	"argument function",         	NULL},

    {"ip",        	1,1,'C',    "args-ip",        	"",         				NULL},
    {"string",      1,1,'C',    "args-string",      "",         				NULL},
    {"numeric",     1,1,'C',    "args-numeric",     "",         				NULL},
    {"command",     1,1,'C',    "args-command",     "",         				NULL},

    {"{arg}",       2,0,'I',    "args-ip-I",        "",         				args_ip},
    {"{arg}",      	2,0,'S',    "args-string-S",    "",         				args_string},
    {"{arg}",     	2,0,'N',    "args-numeric-N",   "",         				args_numeric},
    {"arg",     	2,0,'C',    "args-command-arg", "",         				args_command},

    {"",-1,0,0,"","",NULL}      /* <-- Remain this - critical issue (EOF) */
};
