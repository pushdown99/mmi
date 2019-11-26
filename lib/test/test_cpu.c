#include <stdio.h>

int main()
{
    int ncpu,m1,m5,m15,h1;

    getcpuuse(&ncpu,&m1,&m5,&m15,&h1);
    printf("CPU %d: %d %d %d %d \n",ncpu,m1,m5,m15,h1);
}

