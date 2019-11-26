#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <sys/poll.h>

#ifdef SunOS

#include <fcntl.h>
#include <stropts.h>
#include <sys/types.h>
#include <kvm.h>
#include <sys/fcntl.h>
#include <kstat.h>
#include <errno.h>
#include <time.h>

#include <sys/sockio.h>
#include <sys/socket.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/tihdr.h>
#include <sys/tiuser.h>
#include <inet/common.h>
#include <inet/common.h>
#include <inet/mib2.h>
#include <inet/ip.h>
#include <net/if.h>
#include <netinet/in.h>

#include "mib2.h"

#include "log.h"

#define Line printf("%s:%d \n",__FILE__,__LINE__)

int getkvstat(char* sname,char* vname,void *val)
{
	kstat_ctl_t *kc=NULL;
	kstat_t   *ks, *kd;
	kstat_named_t *d;
	char modname[64];
	int i,instance;
	void *v;

	v = val;

	if(!(kc=kstat_open()))  return 0;
	if(!(ks=kstat_lookup(kc,"unix",0,"kstat_headers")))	goto kstat_fail;
	if(kstat_read(kc,ks,0)<=0) 							goto kstat_fail;
	kd = ks->ks_data;
	
	/* Now, look for the name of our stat in the headers buf */
	for(i=0;i<ks->ks_ndata;i++) {
		if(strcmp(sname, kd[i].ks_name)) continue;
		strcpy(modname, kd[i].ks_module);
		instance = kd[i].ks_instance;
		break;
	}	
	if(i==ks->ks_ndata) 								goto kstat_fail;

	if(!(ks=kstat_lookup(kc,modname,instance,sname))) 	goto kstat_fail;
	if(kstat_read(kc,ks,0)<=0) 							goto kstat_fail;
	if(ks->ks_type != KSTAT_TYPE_NAMED)					goto kstat_fail; 
	for(i=0,d=KSTAT_NAMED_PTR(ks);i<ks->ks_ndata;i++,d++) {
		if(strcmp(d->name,vname)) continue;
		switch (d->data_type) {
		case KSTAT_DATA_CHAR:	
			*(char *)v = (int)d->value.c;
			break;
		case KSTAT_DATA_INT32:	
			*(Counter *)v = d->value.i32;
			break;
		case KSTAT_DATA_UINT32:
			*(Counter *)v = d->value.ui32;
			break;
		case KSTAT_DATA_INT64:
			*(long *)v = d->value.i64;
			break;
		case KSTAT_DATA_UINT64:
			*(unsigned long *)v = d->value.ui64;
			break;
		case KSTAT_DATA_FLOAT:
			*(float *)v = d->value.f;
			break;
		case KSTAT_DATA_DOUBLE:
			*(double *)v = d->value.d;
			break;
		default:
			goto kstat_fail; 
		}
		goto kstat_succ;
	}
kstat_succ:
	kstat_close(kc);
	return 1;

kstat_fail:
	kstat_close(kc);
	return 0;
}

int get_mib2_if(char *dat,int maxlen)
{
	struct ifconf ifc;
	struct ifreq *ifr;
	mib2_ifEntry_t *ifp;
	char buf[BUFSIZ];
	int fd,len=0,i,idx;
	int nentries=maxlen/sizeof(mib2_ifEntry_t);
	
	if((fd=socket(AF_INET,SOCK_DGRAM,0))<0)					return 0;
	ifc.ifc_buf	= buf;
	ifc.ifc_len	= maxlen;
	if(ioctl(fd,SIOCGIFCONF,&ifc)<0) 						goto mib2_if_fail;
	for(i=0,idx=0,ifp=(mib2_ifEntry_t *)dat,ifr=ifc.ifc_req;
		((char *)ifr<((char *)ifc.ifc_buf+ifc.ifc_len)) && (i < nentries);
		i++, ifp++, ifr++, idx++) {

		if(ioctl(fd,SIOCGIFFLAGS,ifr)<0) 					goto mib2_if_fail;
#if 0
		if(!strcmp(ifr->ifr_name,"lo0"))					continue;
#endif

		bzero(ifp,sizeof(mib2_ifEntry_t));
		ifp->ifIndex			= idx;
		ifp->ifDescr.o_length	= strlen(ifr->ifr_name);
		strcpy(ifp->ifDescr.o_bytes, ifr->ifr_name);
		ifp->ifAdminStatus		= (ifr->ifr_flags & IFF_RUNNING) ? 1:2;
		ifp->ifOperStatus		= (ifr->ifr_flags & IFF_UP)? 1:2;
		ifp->ifLastChange		= 0;
		if(ioctl(fd,SIOCGIFMTU,ifr)<0) 						goto mib2_if_fail;
		ifp->ifMtu				= ifr->ifr_metric;	
		ifp->ifType				= 1;
		ifp->ifSpeed			= 0;
		if(getkvstat(ifr->ifr_name,"ifspeed",&ifp->ifSpeed)) {
			if(ifp->ifSpeed<10000) ifp->ifSpeed *= 1000000;
		}
		else getkvstat(ifr->ifr_name,"ifSpeed",&ifp->ifSpeed);
		switch (ifr->ifr_name[0]) {
		case 'l': /* le / lo / lane (ATM LAN Emulation) */
			if(ifr->ifr_name[1] == 'o') {
				if(!ifp->ifSpeed) ifp->ifSpeed = 127000000;
				ifp->ifType = 24;
			} else if(ifr->ifr_name[1] == 'e') {
				if(!ifp->ifSpeed) ifp->ifSpeed = 10000000;
				ifp->ifType = 6;
			} else if(ifr->ifr_name[1] == 'a') {
				if(!ifp->ifSpeed) ifp->ifSpeed = 155000000;
				ifp->ifType = 37;
			}
			break;
		case 'h': /* hme (SBus card) */
		case 'e': /* eri (PCI card) */
		case 'b': /* be */
		case 'd': /* dmfe -- found on netra X1 */
			if(!ifp->ifSpeed) ifp->ifSpeed = 100000000;
			ifp->ifType = 6;
			break;
		case 'f': /* fa (Fore ATM */
			if(!ifp->ifSpeed) ifp->ifSpeed = 155000000;
			ifp->ifType = 37;
			break;	
		case 'q': /* qe (QuadEther) / qa (Fore ATM) / qfe (QuadFastEther) */
			if(ifr->ifr_name[1] == 'a') {
				if(!ifp->ifSpeed) ifp->ifSpeed = 155000000;
				ifp->ifType = 37;
			} else if(ifr->ifr_name[1] == 'e') {
				if(!ifp->ifSpeed) ifp->ifSpeed = 10000000;
				ifp->ifType = 6;
			} else if(ifr->ifr_name[1] == 'f') {
				if(!ifp->ifSpeed) ifp->ifSpeed = 100000000;
				ifp->ifType = 6;
			}
			break;
		}
		if(!strchr (ifr->ifr_name, ':')) {
			Counter l_tmp;
			if(!getkvstat(ifr->ifr_name,"ipackets",&ifp->ifInUcastPkts))		goto mib2_if_fail;
			if(!getkvstat(ifr->ifr_name,"rbytes",&ifp->ifInOctets))
				ifp->ifInOctets 	= ifp->ifInUcastPkts * 308;
			if(!getkvstat(ifr->ifr_name,"opackets",&ifp->ifOutUcastPkts))		goto mib2_if_fail;
			if(!getkvstat(ifr->ifr_name,"obytes",&ifp->ifOutOctets))
				ifp->ifOutOctets 	= ifp->ifOutUcastPkts * 308;
			if(ifp->ifType == 24) continue;
			if(!getkvstat(ifr->ifr_name,"ierrors",&ifp->ifInErrors))			goto mib2_if_fail;
			if(!getkvstat(ifr->ifr_name,"oerrors",&ifp->ifOutErrors))			goto mib2_if_fail;

			if(getkvstat(ifr->ifr_name,"brdcstrcv",&ifp->ifInNUcastPkts)
				&& getkvstat(ifr->ifr_name,"multircv",&l_tmp))
				ifp->ifInNUcastPkts += l_tmp;

			if(getkvstat(ifr->ifr_name,"brdcstxmt",&ifp->ifOutNUcastPkts)
				&& getkvstat(ifr->ifr_name,"multixmt",&l_tmp))
				ifp->ifOutNUcastPkts += l_tmp;
		}
	}
mib2_if_succ:
	close(fd);
	return idx*sizeof(mib2_ifEntry_t);

mib2_if_fail:
	close(fd);
	return 0;
}
#endif

#ifdef Linux

#include <net/if.h>
#include "mib2.h"

int get_mib2_if(char *dat,int maxlen)
{
	char buf[BUFSIZ], line[256], ifname_buf[64], *ifname, *p;
    char *stats, *ifstart;
	struct ifreq *ifr;
	struct ifconf ifc;
	mib2_ifEntry_t *ifp;
	FILE *fp;
	unsigned long rec_pkt, rec_oct, rec_err, rec_dis, snd_pkt, snd_oct, snd_err, snd_dis,coll;
	int i, idx, fd, nentries=maxlen/sizeof(mib2_ifEntry_t);

	const char *scan_line_2_2="%lu %lu %lu %lu %*lu %*lu %*lu %*lu %lu %lu %lu %lu %*lu %lu";
	const char *scan_line_2_0="%lu %lu %*lu %*lu %*lu %lu %lu %*lu %*lu %lu";
	const char *scan_line_to_use;


logprn(0, 0, 0, "(STI-LIB:interface.c) func=get_mib2_if() start \n");

	if (dat==NULL) return 0;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		return -1;
	}

	if (!(fp=fopen("/proc/net/dev", "r"))) {
logprn(0, 0, 0, "(STI-LIB:interface.c) func=get_mib2_if() fopen fail \n");
		close(fd);
        return -1;
	}

    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);

    if (strstr(line, "compressed")) {
	      scan_line_to_use = scan_line_2_2;
	} else {
	      scan_line_to_use = scan_line_2_0;
    } 

	ifc.ifc_buf = buf;
    ifc.ifc_len = maxlen;
    if(ioctl(fd,SIOCGIFCONF,&ifc)<0) goto mib2_if_fail;

	for(i=0,idx=0,ifp=(mib2_ifEntry_t *)dat,ifr=ifc.ifc_req;
        ((char *)ifr<((char *)ifc.ifc_buf+ifc.ifc_len)) && (i < nentries) 
		&& fgets(line, sizeof(line), fp);
	    i++, ifp++, ifr++, idx++) {

		ifstart = line;

		ifp->ifIndex=0;
	    bzero(ifp, sizeof(mib2_ifEntry_t));

		ifp->ifIndex=0;
        if (line[strlen(line)-1]  == '\n')
            line[strlen(line)-1]='\0';

        while((*ifstart) == ' ') ifstart++;

        if ((stats=strrchr(ifstart, ':')) == NULL)
            continue;

	    if((scan_line_to_use == scan_line_2_2) && ((stats-line) <6))
	        continue; /* -_-; */

        *stats++ = 0;
        strcpy(ifname_buf, ifstart);

        if(strncmp(ifname_buf, "eth",3))   {
            idx--; ifp--; ifr--;
logprn(0, 0, 0, "(STI-LIB:interface.c) func=get_mib2_if() ifname=%s continue \n", ifname_buf);
            continue;   
        }

        while((*stats)== ' ') *stats++;

        if ((scan_line_to_use == scan_line_2_2 
				&& sscanf(stats, scan_line_to_use, &rec_oct, &rec_pkt, &rec_err, &rec_dis,&snd_oct, &snd_pkt, &snd_err, &snd_dis,&coll) != 9) 
		   || (scan_line_to_use == scan_line_2_0 
			 	&& sscanf(stats, scan_line_to_use, &rec_pkt, &rec_err, &rec_dis, &snd_pkt, &snd_err, &rec_dis, &coll) !=7)) {
				if ((scan_line_to_use==scan_line_2_2) && !strstr(line, "No statics available"))
						printf("\n Error!");
				continue;
		} 
	
		*((int *)&ifp->ifIndex)	= idx;
		*((Counter *)&ifp->ifInOctets) = rec_oct;
		*((Counter *)&ifp->ifInUcastPkts) = rec_pkt;
		*((Counter *)&ifp->ifInNUcastPkts) = 0; 
		*((Counter *)&ifp->ifInDiscards) = rec_dis; 
		*((Counter *)&ifp->ifInErrors) = rec_err; 
		*((Counter *)&ifp->ifOutOctets) = snd_oct; 
		*((Counter *)&ifp->ifOutUcastPkts) = snd_pkt; 
		*((Counter *)&ifp->ifOutNUcastPkts) = 0; 
		*((Counter *)&ifp->ifOutDiscards) = snd_dis; 
		*((Counter *)&ifp->ifOutErrors) = snd_err;


		if(ioctl(fd,SIOCGIFFLAGS,ifr)<0) 					goto mib2_if_fail;
#if 0
		if(!strcmp(ifr->ifr_name,"lo0"))					continue;
#endif

		ifp->ifIndex			= idx;
		ifp->ifDescr.o_length	= strlen(ifr->ifr_name);
		strcpy(ifp->ifDescr.o_bytes, ifr->ifr_name);
		ifp->ifAdminStatus		= (ifr->ifr_flags & IFF_RUNNING) ? 1:2;
		ifp->ifOperStatus		= (ifr->ifr_flags & IFF_UP)? 1:2;
		ifp->ifLastChange		= 0;
		if(ioctl(fd,SIOCGIFMTU,ifr)<0) 						goto mib2_if_fail;
		ifp->ifMtu				= ifr->ifr_metric;	
		ifp->ifType				= 1;
		ifp->ifSpeed			= 0;
#if 0
		if(getkvstat(ifr->ifr_name,"ifspeed",&ifp->ifSpeed)) {
			if(ifp->ifSpeed<10000) ifp->ifSpeed *= 1000000;
		}
		else getkvstat(ifr->ifr_name,"ifSpeed",&ifp->ifSpeed);
#endif
		switch (ifr->ifr_name[0]) {
		case 'l': /* le / lo / lane (ATM LAN Emulation) */
			if(ifr->ifr_name[1] == 'o') {
				if(!ifp->ifSpeed) ifp->ifSpeed = 127000000;
				ifp->ifType = 24;
			} else if(ifr->ifr_name[1] == 'e') {
				if(!ifp->ifSpeed) ifp->ifSpeed = 10000000;
				ifp->ifType = 6;
			} else if(ifr->ifr_name[1] == 'a') {
				if(!ifp->ifSpeed) ifp->ifSpeed = 155000000;
				ifp->ifType = 37;
			}
			break;
		case 'h': /* hme (SBus card) */
		case 'e': /* eri (PCI card) */
		case 'b': /* be */
		case 'd': /* dmfe -- found on netra X1 */
			if(!ifp->ifSpeed) ifp->ifSpeed = 100000000;
			ifp->ifType = 6;
			break;
		case 'f': /* fa (Fore ATM */
			if(!ifp->ifSpeed) ifp->ifSpeed = 155000000;
			ifp->ifType = 37;
			break;	
		case 'q': /* qe (QuadEther) / qa (Fore ATM) / qfe (QuadFastEther) */
			if(ifr->ifr_name[1] == 'a') {
				if(!ifp->ifSpeed) ifp->ifSpeed = 155000000;
				ifp->ifType = 37;
			} else if(ifr->ifr_name[1] == 'e') {
				if(!ifp->ifSpeed) ifp->ifSpeed = 10000000;
				ifp->ifType = 6;
			} else if(ifr->ifr_name[1] == 'f') {
				if(!ifp->ifSpeed) ifp->ifSpeed = 100000000;
				ifp->ifType = 6;
			}
			break;
		}
	}

mib2_if_succ:
logprn(0, 0, 0, "(STI-LIB:interface.c) func=get_mib2_if() success \n");
	fclose(fp);
    close(fd);
    return idx*sizeof(mib2_ifEntry_t);

mib2_if_fail:
logprn(0, 0, 0, "(STI-LIB:interface.c) func=get_mib2_if() fail \n");
	fclose(fp);
    close(fd);
    return 0;
}

#endif

void prn_mib2_if(char *s,int len)
{
	char *p = s;
	int i;
	int nentries=len/sizeof(mib2_ifEntry_t);

	for(i=0;i<nentries;i++) {
		printf("ifIndex				: %d \n", (*((unsigned int*)p))++);
		p = p + sizeof(int);
		printf("ifDescr				: %s \n",p); p+=32;
		printf("ifType				: %d \n", (*((unsigned int*)p))++);
		printf("ifMtu				: %d \n", (*((unsigned int*)p))++);
		printf("ifSpeed				: %d \n", (*((unsigned int*)p))++);
		p = p + sizeof(int);
		printf("ifPhysAddress		: %s \n",p); p+=32;
		printf("ifAdminStatus		: %d \n", (*((unsigned int*)p))++);
		printf("ifOperStatus		: %d \n", (*((unsigned int*)p))++);
		printf("ifLastChange		: %d \n", (*((unsigned int*)p))++);
		printf("ifInOctets			: %lu \n",(*((unsigned long*)p))++);
		printf("ifInUcastPkts		: %lu \n",(*((unsigned long*)p))++);
		printf("ifInNUcastPkts		: %lu \n",(*((unsigned long*)p))++);
		printf("ifInDiscards		: %lu \n",(*((unsigned long*)p))++);
		printf("ifInErrors			: %lu \n",(*((unsigned long*)p))++);
		printf("ifInUnknownProtos	: %lu \n",(*((unsigned long*)p))++);
		printf("ifOutOctets			: %lu \n",(*((unsigned long*)p))++);
		printf("ifOutUcastPkts		: %lu \n",(*((unsigned long*)p))++);
		printf("ifOutNUcastPkts		: %lu \n",(*((unsigned long*)p))++);
		printf("ifOutDiscards		: %lu \n",(*((unsigned long*)p))++);
		printf("ifOutErrors			: %lu \n",(*((unsigned long*)p))++);
		printf("ifOutQLen			: %lu \n",(*((unsigned long*)p))++);
		printf("ifSpecific			: %lu \n",(*((unsigned long*)p))++);
	}
}


#if defined(STANDALONE)

int main(int argc, char* argv[])
{

	char dat[BUFSIZ];
	int nbyte;

	nbyte = get_mib2_if(dat,BUFSIZ);
	prn_mib2_if(dat,nbyte);
}

#endif
