#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stropts.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/ioctl.h>

#ifdef SunOS
#include <sys/sockio.h>
#endif

#ifdef Linux
#include <linux/sockios.h>
#endif

#include "__if.h"

#ifdef Linux
#include "ioctl_func.h"
#endif

if_t	ifs[MAXIF];

/* for thread_safe_func ( getifs(..) )  2004/10/06 chang-woo    */
pthread_mutex_t bufq_ifs_mutex = PTHREAD_MUTEX_INITIALIZER;
#define IFS_LOCK    pthread_mutex_lock(&bufq_ifs_mutex)
#define IFS_UNLOCK  pthread_mutex_unlock(&bufq_ifs_mutex)

#ifdef Linux

unsigned char*
getmac(unsigned char* ifname,unsigned char* eth)
{
	sioc_gifhwaddr(ifname, eth);
	return eth;
}
#endif

#ifdef SunOS

unsigned char*
getmac(unsigned char* ifname,unsigned char* eth)
{
    int i;

	getifs();
	for(i=0;ifs[i].fld[IF_NAME_FLD][0];i++) {
		if(strcmp(ifs[i].fld[IF_NAME_FLD],(char*)ifname)) continue;
		memcpy(eth,ifs[i].mac,6);
		return eth;
	}
	return NULL;
}
#endif

unsigned int
getip(char* ifname)
{
	unsigned int addr;
	addr = sioc_gifaddr(ifname);
	return addr;
}

#ifdef Linux

unsigned int
getmtu(char* ifname)
{
	int mtu;
	mtu = sioc_gifmtu(ifname);
    return mtu;
}

void
setmtu(char* ifname,unsigned int mtu)
{
	sioc_sifmtu(ifname, mtu);
}
#endif

#ifdef SunOS

unsigned int
getmtu(unsigned char* ifname)
{
    int i;

	getifs();
	for(i=0;ifs[i].fld[IF_NAME_FLD][0];i++) {
		if(strcmp(ifs[i].fld[IF_NAME_FLD],(char*)ifname)) continue;
		return ifs[i].mtu;
	}
	return 0;
}

void
setmtu(unsigned char* ifname,unsigned int mtu)
{
	char cmd[BUFSIZ];
	sprintf(cmd,"/usr/sbin/ifconfig %s mtu %d",ifname,mtu);
	system(cmd);
}
#endif

#ifdef Linux

void ifdown(unsigned char* ifname)
{
	unsigned short flags;
	flags = sioc_gifflags(ifname);
	flags &= ~IFF_UP;
	sioc_sifflags(ifname, flags);
}

void ifup(unsigned char* ifname, unsigned int addr, unsigned int mask, unsigned int bcast)
{
	unsigned short flags;
	flags = sioc_gifflags(ifname);
	if (addr) 
		sioc_sifaddr(ifname, addr);
	if (mask)
		sioc_sifnetmask(ifname, mask);
	if (bcast)
		sioc_sifbrdaddr(ifname, bcast);
	flags |= IFF_UP;
	sioc_sifflags(ifname, flags);
}
#endif

#ifdef SunOS

void ifdown(unsigned char* ifname)
{
	char cmd[BUFSIZ];
	sprintf(cmd,"/usr/sbin/ifconfig %s down",ifname);
	system(cmd);
}

void ifup(unsigned char* ifname)
{
	char cmd[BUFSIZ];
	sprintf(cmd,"/usr/sbin/ifconfig %s up",ifname);
	system(cmd);
}
#endif

#ifdef Linux
#endif

#ifdef SunOS

#define IFMODE	"/tmp/mode"
#define IFSPEED	"/tmp/speed"

int getdevinst(char* dev,char* device,int *inst)
{
	if(!strncasecmp(dev,"hme",3)) 		{ *inst = atoi(&dev[3]); strcpy(device,"hme");	}
	else if(!strncasecmp(dev,"eri",3)) 	{ *inst = atoi(&dev[3]); strcpy(device,"eri");	}
	else if(!strncasecmp(dev,"dmfe",4)) { *inst = atoi(&dev[4]); strcpy(device,"dmfe");	}
	else if(!strncasecmp(dev,"qfe",3)) 	{ *inst = atoi(&dev[3]); strcpy(device,"qfe");	}
	else return -1;

	return 1;
}
int getifval(char* fname)
{
	int val;

	FILE* fp;
	if(!(fp=fopen(fname,"r"))) return 0;
	fscanf(fp,"%d",&val);
	fclose(fp);
	return val;
}

int getifmode(char* dev)
{
	struct stat b;
	char path[BUFSIZ],cmd[BUFSIZ];
	char device[BUFSIZ];
	int inst;
	
	sprintf(path,"/dev/%s",dev);
	if(stat(path,&b)) {
		getdevinst(dev,device,&inst);
		sprintf(cmd,"/usr/sbin/ndd -set /dev/%s instance %d",device,inst); system(cmd);
	}
	else strcpy(device,dev);
	sprintf(cmd,"/usr/sbin/ndd -get /dev/%s link_mode > %s",device,IFMODE);
	return getifval(IFMODE);
}
int getifspeed(char* dev)
{
	struct stat b;
	char path[BUFSIZ],cmd[BUFSIZ];
	char device[BUFSIZ];
	int inst;
	
	sprintf(path,"/dev/%s",dev);
	if(stat(path,&b)) {
		getdevinst(dev,device,&inst);
		sprintf(cmd,"/usr/sbin/ndd -set /dev/%s instance %d",device,inst); system(cmd);
	}
	else strcpy(device,dev);
	sprintf(cmd,"/usr/sbin/ndd -get /dev/%s link_speed > %s",device,IFSPEED);
	return getifval(IFSPEED);
}

#define NEGO_100H	0x01
#define NEGO_100F	0x02
#define NEGO_10H	0x04
#define NEGO_10F	0x08
#define NEGO_AUTO	0x10

void setifduplex(char* dev,char* duplex,int speed)
{
	struct stat b;
	char path[BUFSIZ],cmd[BUFSIZ];
	char device[BUFSIZ];
	int inst,flags;
	
	sprintf(path,"/dev/%s",dev);
	if(stat(path,&b)) {
		getdevinst(dev,device,&inst);
		sprintf(cmd,"/usr/sbin/ndd -set /dev/%s instance %d",device,inst); system(cmd);
	}
	else strcpy(device,dev);

	flags = 0;
	if(!strcasecmp(duplex,"auto")) flags |= NEGO_AUTO;
	if(!strcasecmp(duplex,"half")) {
		if(speed==10) 	flags |= NEGO_10H;
		if(speed==100) 	flags |= NEGO_100H;
	}
	if(!strcasecmp(duplex,"full")) {
		if(speed==10) 	flags |= NEGO_10F;
		if(speed==100) 	flags |= NEGO_100F;
	}
	if(!flags) return;
    sprintf(cmd,"ndd -set /dev/%s adv_100fdx_cap %d",	device,(flags&NEGO_100F)?1:0); 	system(cmd);
    sprintf(cmd,"ndd -set /dev/%s adv_100hdx_cap %d",	device,(flags&NEGO_100H)?1:0); 	system(cmd);
    sprintf(cmd,"ndd -set /dev/%s adv_10hdx_cap %d",	device,(flags&NEGO_10H )?1:0); 	system(cmd);
    sprintf(cmd,"ndd -set /dev/%s adv_10fdx_cap %d",	device,(flags&NEGO_10F )?1:0); 	system(cmd);
    sprintf(cmd,"ndd -set /dev/%s adv_autoneg_cap %d",	device,(flags&NEGO_AUTO)?1:0); 	system(cmd);
}
#endif

/* this function must be deprecated. (ioctl_func.c sioc_gifflags()) */
unsigned int
getifflags(char* ifname)
{
    struct ifreq ifr;
    int fd;
 
    if((fd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))<0) return 0;

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name,(char*)ifname);
    if(ioctl(fd,SIOCGIFFLAGS,&ifr)<0) {
        perror("SIOCGIFFLAGS");
        close(fd);
        return 0;
    }
    close(fd);
    return ifr.ifr_flags;
}

void prnifflags(char *ifn,int flags)
{
    printf("Interface: %s \n",ifn);
    printf("Flags    : ");
    if(flags & IFF_UP)          printf("|UP");
    if(flags & IFF_BROADCAST)   printf("|BROADCAST");
    if(flags & IFF_LOOPBACK)    printf("|LOOPBACK");
    if(flags & IFF_POINTOPOINT) printf("|POINTOPOINT");
    if(flags & IFF_RUNNING)     printf("|RUNNING");
    if(flags & IFF_MULTICAST)   printf("|MULTICAST");
    printf("\n\n");
}


#define _IFNAME		0x00000001
#define _IFMTU 		0x00000002
#define _IFMAC 		0x00000004
#define _IFADDR		0x00000008
#define _IFMASK		0x00000010
#define _IFBCAST	0x00000020
#define _IFCLEAR	0x00000040

#define _IFUFLAG	0x00010000
#define _IFLFLAG	0x00020000
#define _IFRFLAG	0x00040000
#define _IFMFLAG	0x00080000
#define _IFBFLAG	0x00100000

#define IFFILE	"/tmp/ifs"

char* getiff(int iff)
{
	switch(iff) {
	case _IFUFLAG	: return (char*)"Up";
	case _IFLFLAG	: return (char*)"Loop";
	case _IFRFLAG	: return (char*)"Run";
	case _IFBFLAG	: return (char*)"Bcast";
	case _IFMFLAG	: return (char*)"Mcast";
	}
	return NULL;
}

#ifdef Linux

int getiftok(char *p)
{
#if 0
	if(!strncmp(p,"lo",2)) 			return _IFNAME;
#endif
	if(!strncmp(p,"eth",3)) 		return _IFNAME;
	if(!strncmp(p,"bond",4)) 		return _IFNAME;
	if(!strncmp(p,"MTU:",4)) 		return _IFMTU;
	if(!strncmp(p,"HWaddr",6)) 		return _IFMAC;
	if(!strncmp(p,"addr:",5)) 		return _IFADDR;
	if(!strncmp(p,"Mask:",5)) 		return _IFMASK;
	if(!strncmp(p,"Bcast:",6)) 		return _IFBCAST;

	if(!strncmp(p,"Interrupt:",10))	return _IFCLEAR;
	if(!strncmp(p,"RX",2))			return _IFCLEAR;
	if(!strncmp(p,"TX",2))			return _IFCLEAR;

	if(!strcmp(p,"UP")) 			return _IFUFLAG;
	if(!strcmp(p,"LOOPBACK")) 		return _IFLFLAG;
	if(!strcmp(p,"BROADCAST")) 		return _IFBFLAG;
	if(!strcmp(p,"MULTICAST")) 		return _IFMFLAG;
	if(!strcmp(p,"RUNNING")) 		return _IFRFLAG;
	return 0;
}

int getifspeed(char *dev)
{
	struct ifreq ifr;
	struct ethtool_cmd ecmd;
	struct ethtool_wolinfo wolinfo;
	struct ethtool_value edata;
	int ret, fd, err;

    /* Setup our control structures. */
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, dev);

    /* Open control socket. */
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("Cannot get control socket");
   	 	return 0;
    }

	ecmd.cmd = ETHTOOL_GSET;
	ifr.ifr_data = (caddr_t)&ecmd;
	err = ioctl(fd, SIOCETHTOOL, &ifr);
	if (err == 0) {
		switch(ecmd.speed) {
		case SPEED_10	: ret=1; break;
		case SPEED_100	: ret=2; break;
		case SPEED_1000	: ret=3; break; 
		case SPEED_10000: ret=4; break;
		default			: ret=0; break; 
		}

		close(fd);
		return ret;

	} 

	close(fd);
	return 0;
}

int getifmode(char *dev)
{
	struct ifreq ifr;
	struct ethtool_cmd ecmd;
	struct ethtool_wolinfo wolinfo;
	struct ethtool_value edata;
	int fd, err;

    /* Setup our control structures. */
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, dev);

    /* Open control socket. */
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("Cannot get control socket");
   	 	return 0;
    }

	ecmd.cmd = ETHTOOL_GSET;
	ifr.ifr_data = (caddr_t)&ecmd;
	err = ioctl(fd, SIOCETHTOOL, &ifr);
	if (err == 0) {
		close(fd);
		return (ecmd.duplex+1);
	}
		
	close(fd);
	return 0;
}


int getifstat(char *dev)
{
	struct ifreq ifr;
	struct ethtool_cmd ecmd;
	struct ethtool_wolinfo wolinfo;
	struct ethtool_value edata;
	int fd, err;

    /* Setup our control structures. */
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, dev);

    /* Open control socket. */
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("Cannot get control socket");
   	 	return 0;
    }

	edata.cmd = ETHTOOL_GLINK;
	ifr.ifr_data = (caddr_t)&edata;
	err = ioctl(fd, SIOCETHTOOL, &ifr);
	if (err == 0)  {
		close(fd);
		return edata.data;
	}

	close(fd);
	return 0;
}

int getifs()
{
	FILE *fp;
	struct stat b;
	int n,iff,iffs,val,nif=-1;
	int mbl,i;
	unsigned int _nbit;
	char cmd[BUFSIZ],arg1[BUFSIZ],arg2[BUFSIZ];
	char *cp=NULL,*tp, *p,*ptr;

    IFS_LOCK;   /* for thread_safe  */ 

	bzero(&ifs[0],sizeof(if_t)*MAXIF);

	sprintf(cmd,"ifconfig -a > %s",IFFILE);
	system(cmd);
	if(stat(IFFILE,&b)) { IFS_UNLOCK; return 0; }
	if(!(fp=fopen(IFFILE,"r"))) { IFS_UNLOCK;   return 0;   }
	if(!(cp=(char*)malloc(b.st_size))) goto err_ifs;
	if((n=fread(cp,1,b.st_size,fp))<=0) goto err_ifs;
	fclose(fp);

	for(p=strtok_r(cp," \t\n", &tp);p;p=strtok_r(NULL," \t\n", &tp)) {
		if(!(iff=getiftok(p))) continue;
			
		iffs |= iff;
		if(!(iffs&_IFNAME)) continue;

		switch(iff) {
		case _IFNAME	:
			nif+=1;
			iffs = iff;
			strcpy(ifs[nif].fld[IF_NAME_FLD],p);
			break;
		case _IFMAC		:
			bzero(arg1, BUFSIZ); 	bzero(arg2, BUFSIZ);
			p=strtok_r(NULL," \t\n", &tp);
			strcpy(ifs[nif].fld[IF_MAC_FLD],p);
			strcpy(arg1,p);
			while(ptr=strchr(arg1,':')) *ptr = ' ';
			sscanf(arg1,"%02x%02x%02x%02x%02x%02x"
				,(unsigned int*)&ifs[nif].mac[0]
				,(unsigned int*)&ifs[nif].mac[1]
				,(unsigned int*)&ifs[nif].mac[2]
				,(unsigned int*)&ifs[nif].mac[3]
				,(unsigned int*)&ifs[nif].mac[4]
				,(unsigned int*)&ifs[nif].mac[5]
			);
			break;
		case _IFADDR	:
			if(ptr=strchr(p,':')) *ptr=' ';
			bzero(arg1, BUFSIZ); 	bzero(arg2, BUFSIZ);
			sscanf(p,"%s%s",arg1,arg2);
			if(strlen(arg2)<=0) break;
			strcpy(ifs[nif].fld[IF_ADDR_FLD],arg2);
			ifs[nif].addr = inet_addr(arg2);
			break;
		case _IFBCAST	:
			bzero(arg1, BUFSIZ); 	bzero(arg2, BUFSIZ);
			if(ptr=strchr(p,':')) *ptr=' ';
			sscanf(p,"%s%s",arg1,arg2);
			if(strlen(arg2)<=0) break;
			strcpy(ifs[nif].fld[IF_BCAST_FLD],arg2);
			ifs[nif].bcast = inet_addr(arg2);
			break;
		case _IFMASK	:
			bzero(arg1, BUFSIZ); 	bzero(arg2, BUFSIZ);
			if(ptr=strchr(p,':')) *ptr=' ';
			sscanf(p,"%s%s",arg1,arg2);
			if(strlen(arg2)<=0) break;
			strcpy(ifs[nif].fld[IF_MASK_FLD],arg2);
			ifs[nif].mask = inet_addr(arg2);
			_nbit = htonl(inet_addr(arg2));
			for(mbl=0;mbl<=32;mbl++) {
				if(_nbit<<mbl) continue;
				ifs[nif].mbl=mbl;
				break;
			}
			break;
		case _IFUFLAG	:
			ifs[nif].u	= 1;
			break;
		case _IFMFLAG	:
			ifs[nif].m	= 1;
			break;
		case _IFBFLAG	:
			ifs[nif].b	= 1;
			break;
		case _IFLFLAG	:
			ifs[nif].l	= 1;
			break;
		case _IFRFLAG	:
			ifs[nif].r	= 1;
			break;
		case _IFMTU		:
			bzero(arg1, BUFSIZ); 	bzero(arg2, BUFSIZ);
			if(ptr=strchr(p,':')) *ptr=' ';
			sscanf(p,"%s%d",arg1,&val);
			ifs[nif].mtu	= val;
			break;
		case _IFCLEAR	:
			iffs = 0;
			break;
		}
	}

	if(cp) free(cp);

#if defined(EXAMPLECODE)
	for(i=0;ifs[i].fld[IF_NAME_FLD][0];i++) {
		printf("%s: %s/%d %s (%02x:%02x:%02x:%02x:%02x:%02x)\n"
			,ifs[i].fld[IF_NAME_FLD]
			,ifs[i].fld[IF_ADDR_FLD]
			,ifs[i].mbl
			,ifs[i].fld[IF_BCAST_FLD]
			,ifs[i].mac[0]
			,ifs[i].mac[1]
			,ifs[i].mac[2]
			,ifs[i].mac[3]
			,ifs[i].mac[4]
			,ifs[i].mac[5]
		);
	}
#endif
    IFS_UNLOCK;
	return n;

err_ifs:
	if(cp) free(cp);
	fclose(fp);
    IFS_UNLOCK;
	return 0;
}
#endif

#ifdef SunOS

int getiftok(char *p)
{
	if(!strncmp(p,"hme",3)
		|| !strncmp(p,"eri",3)
		|| !strncmp(p,"qfe",3)) { 		
		if(strlen(p)>5) return 0;
		return _IFNAME;
	}
	if(!strncmp(p,"dmfe",4)) {
		if(strlen(p)>6) return 0;
		return _IFNAME;
	}
	if(!strncmp(p,"mtu",3)) 		return _IFMTU;
	if(!strncmp(p,"ether",5)) 		return _IFMAC;
	if(!strncmp(p,"inet",4)) 		return _IFADDR;
	if(!strncmp(p,"netmask",7)) 	return _IFMASK;
	if(!strncmp(p,"broadcast",9)) 	return _IFBCAST;
	if(!strcmp(p,"UP")) 			return _IFUFLAG;
	if(!strcmp(p,"LOOPBACK")) 		return _IFLFLAG;
	if(!strcmp(p,"BROADCAST")) 		return _IFBFLAG;
	if(!strcmp(p,"MULTICAST")) 		return _IFMFLAG;
	if(!strcmp(p,"RUNNING")) 		return _IFRFLAG;
	return 0;
}

int getifs()
{
	FILE *fp;
	struct stat b;
	int n,iff,iffs,val,nif=-1;
	int mbl,i;
	int d1,d2,d3,d4,d5,d6;
	char cmd[BUFSIZ],arg1[BUFSIZ],arg2[BUFSIZ];
	char *cp=NULL,*tp,*p,*ptr;

    IFS_LOCK;   /* for thread safe  */

	bzero(&ifs[0],sizeof(if_t)*MAXIF);

	sprintf(cmd,"ifconfig -a > %s",IFFILE);
	system(cmd);
	if(stat(IFFILE,&b)) { IFS_UNLOCK;   return 0;   }
	system(cmd);
	if(!(fp=fopen(IFFILE,"r"))) { IFS_UNLOCK;   return 0;   }
	if(!(cp=(char*)malloc(b.st_size))) goto err_ifs;
	if((n=fread(cp,1,b.st_size,fp))<=0) goto err_ifs;
	fclose(fp);

	for(p=strtok_r(cp," \t\n", &tp);p;p=strtok(NULL," \t\n", &tp)) {
		if(!(iff=getiftok(p))) continue;
	
		iffs |= iff;
		if(!(iffs&_IFNAME)) continue;

		switch(iff) {
		case _IFNAME	:
			nif+=1;
			iffs = iff;
			if(ptr=strchr(p,':')) *ptr=0;
			strcpy(ifs[nif].fld[IF_NAME_FLD],p);
			break;
		case _IFMAC		:
			p=strtok_r(NULL," \t\n", &tp);
			strcpy(ifs[nif].fld[IF_MAC_FLD],p);
			strcpy(arg1,p);
			while(ptr=strchr(arg1,':')) *ptr = ' ';
			sscanf(arg1,"%x%x%x%x%x%x"
				,&d1
				,&d2
				,&d3
				,&d4
				,&d5
				,&d6
			);
			ifs[nif].mac[0]=d1;
			ifs[nif].mac[1]=d2;
			ifs[nif].mac[2]=d3;
			ifs[nif].mac[3]=d4;
			ifs[nif].mac[4]=d5;
			ifs[nif].mac[5]=d6;
			iffs=0;
			break;
		case _IFADDR	:
			p=strtok_r(NULL," \t\n", &tp);
			strcpy(ifs[nif].fld[IF_ADDR_FLD],p);
			ifs[nif].addr 	= inet_addr(p);
			break;
		case _IFBCAST	:
			p=strtok_r(NULL," \t\n", &tp);
			strcpy(ifs[nif].fld[IF_BCAST_FLD],p);
			ifs[nif].bcast 	= inet_addr(p);
			break;
		case _IFMASK	:
			p=strtok_r(NULL," \t\n", &tp);
			sscanf(p,"%x",&ifs[nif].mask);
			strcpy(ifs[nif].fld[IF_MASK_FLD]
				,inet_ntoa(*(struct in_addr*)&ifs[nif].mask));
			for(mbl=0;mbl<=32;mbl++) {
				if(ifs[nif].mask<<mbl) continue;
				ifs[nif].mbl=mbl; break;
			}
			break;
		case _IFUFLAG	:
			ifs[nif].u	= 1;
			break;
		case _IFMFLAG	:
			ifs[nif].m	= 1;
			break;
		case _IFBFLAG	:
			ifs[nif].b	= 1;
			break;
		case _IFLFLAG	:
			ifs[nif].l	= 1;
			break;
		case _IFRFLAG	:
			ifs[nif].r	= 1;
			break;
		case _IFMTU		:
			p=strtok_r(NULL," \t\n", &tp);
			sscanf(p,"%d",&val);
			ifs[nif].mtu	= val;
			break;
		case _IFCLEAR	:
			iffs = 0;
			break;
		}
	}

	free(cp);

#ifdef VERBOSE
	for(i=0;ifs[i].fld[IF_NAME_FLD][0];i++) {
		printf("%s: %s/%d %s (%02x:%02x:%02x:%02x:%02x:%02x)\n"
			,ifs[i].fld[IF_NAME_FLD]
			,ifs[i].fld[IF_ADDR_FLD]
			,ifs[i].mbl
			,ifs[i].fld[IF_BCAST_FLD]
			,ifs[i].mac[0]
			,ifs[i].mac[1]
			,ifs[i].mac[2]
			,ifs[i].mac[3]
			,ifs[i].mac[4]
			,ifs[i].mac[5]
		);
	}
#endif
    IFS_UNLOCK;
	return n;

err_ifs:
	if(cp) free(cp);
	fclose(fp);
    IFS_UNLOCK;
	return 0;
}
#endif

#ifdef Linux

/* this function must be deprecated. (ioctl_func.c sioc_gifindex()) */
int
getifindex(unsigned char* ifname)
{
	struct ifreq ifr;
	int fd;

	if((fd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))<0) return -1;
	strcpy(ifr.ifr_name,ifname);
	if(ioctl(fd,SIOCGIFINDEX,&ifr)<0) {
			perror("SIOCGIFINDEX");
			close(fd);
			return -1;
	}
	close(fd);
	return ifr.ifr_ifindex;
}
#endif

#if defined(EXAMPLECODE)

int
main()
{
	unsigned char mac[6];

	getifs();

	getmac("hme0",mac);
	printf("mac: %02x-%02x-%02x-%02x-%02x-%02x\n"
		,mac[0]
		,mac[1]
		,mac[2]
		,mac[3]
		,mac[4]
		,mac[5]
	);
	printf("mtu: %d \n",getmtu("hme0"));
}

#endif
