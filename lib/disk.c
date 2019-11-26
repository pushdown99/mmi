#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "disk.h"

#ifdef SunOS

#include <fcntl.h>
#include <sys/types.h>
#include <sys/mnttab.h>
#include <sys/statvfs.h>

int getdiskuse(disk_t dp[])
{
	FILE* fp;
	struct mnttab	mnt;
	struct statvfs	vfs;
	int fd,cnt=0;

	if(!(fp=fopen(MNTTAB,"r"))) return 0;
	while(!getmntent(fp,&mnt)) {
		fflush(fp);
		if(strcmp(mnt.mnt_fstype,"ufs")) continue;
		if((fd=open(mnt.mnt_mountp,O_RDONLY))<0) continue;
		if(!fstatvfs(fd,&vfs)) {
			strcpy(dp[cnt].name,mnt.mnt_mountp);
			/*
		 	* vfs.f_bsize			: fundamental file system block size
		 	* vfs.f_blocks			: total blocks
		 	* vfs.f_bfree			: total free blocks
		 	* vfs.f_bavail			: free blocks avail to non-superuser
		 	*/
			dp[cnt].block	= vfs.f_bsize;
			dp[cnt].total	= vfs.f_blocks;
			dp[cnt].avail	= vfs.f_bavail;
			dp[cnt].free 	= vfs.f_bfree;
			dp[cnt].used 	= vfs.f_blocks-vfs.f_bavail;
			dp[cnt].pcnt	= 100-(vfs.f_bavail*100/vfs.f_blocks);
			cnt++;
		}
		close(fd);
	}
	dp[cnt].name[0]=0;
	fclose(fp);
	return (cnt)? 1:0;
}

#endif

#ifdef Linux

#define DFFILE	"/tmp/df"

int getdiskuse(disk_t *dp)
{
    FILE* fp;
    char s[BUFSIZ],*p;
    char fn[32],name[32],used[32];
    int d1,d2,d3,cnt=0;

    sprintf(s,"df -k > %s",DFFILE);
    system(s);
    if(!(fp=fopen(DFFILE,"r"))) return 0;
    bzero(s,BUFSIZ);
    while(fgets(s,BUFSIZ,fp)) {
        btrim(s); fflush(fp);
        if((p=strchr(s,'\n'))) *p=0;
        if(sscanf(s,"%s%d%d%d%s%s",fn,&d1,&d2,&d3,used,name)==6) {
            if((p=strchr(s,'%'))) *p=0;
            strcpy(dp[cnt].name,name);
            dp[cnt].block   = 1024;
            dp[cnt].total   = d1;
            dp[cnt].avail   = d3;
            dp[cnt].free    = d3;
            dp[cnt].used    = d2;
            dp[cnt].pcnt    = atoi(used);
            cnt++;
        }
        bzero(s,BUFSIZ);
    }
    dp[cnt].name[0]=0;
    fclose(fp);
	return 1;
}

#endif

int getrdiskuse(int *total,int *avail)
{
	disk_t dd[32];
	int i;

	*total = 0;
	*avail= 0;

	getdiskuse(dd);

	for(i=0;dd[i].name[0] && i<32;i++) {
        if(strcmp(dd[i].name, "/"))    continue;
		*total += dd[i].total;
		*avail += dd[i].avail;
        return 1;
	}
	return 0;
}

int gettdiskuse(int *total,int *avail)
{
	disk_t dd[32];
	int i;

	*total = 0;
	*avail = 0;

	getdiskuse(dd);

	for(i=0;dd[i].name[0] && i<32;i++) {
		*total += dd[i].total;
		*avail += dd[i].avail;
	}
	return i;
}

int getrdiskuse2(int *total,int *used)
{
	disk_t dd[32];
	int i;

	*total = 0;
	*used = 0;

	getdiskuse(dd);

	for(i=0;dd[i].name[0] && i<32;i++) {
        if(strcmp(dd[i].name, "/"))    continue;
		*total += dd[i].total;
		*used += dd[i].used;
        return 1;
	}
	return 0;
}

int gettdiskuse2(int *total,int *used)
{
	disk_t dd[32];
	int i;

	*total = 0;
	*used = 0;

	getdiskuse(dd);

	for(i=0;dd[i].name[0] && i<32;i++) {
		*total += dd[i].total;
		*used += dd[i].used;
	}
	return i;
}

