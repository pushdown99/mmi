#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "mmc.h"

void quit(const char* va_alist, ...)
{
	printf("terminated \n");
	exit(0);
}

void start(const char* va_alist, ...)
{
	printf("start \n");
}

void stop(const char* va_alist, ...)
{
	printf("stop \n");
}

void args_ip(const char* va_alist, ...)
{
    DECLARE_VAR;
    GET_VAR_ARG(va_alist);

    printf("address: %s \n",arg3);
}

void args_string(const char* va_alist, ...)
{
    DECLARE_VAR;
    GET_VAR_ARG(va_alist);

    printf("string: %s \n",arg3);
}

void args_numeric(const char* va_alist, ...)
{
	DECLARE_VAR;
    GET_VAR_ARG(va_alist);

    printf("numeric: %d \n",atoi(arg3));
}

void args_command(const char* va_alist, ...)
{
	DECLARE_VAR;
    GET_VAR_ARG(va_alist);

    printf("command: %s \n",arg3);
}


