/*
 * Author : albam <albamc@gmail.com>
 * Copyright 2005 albam
 *
 * Local ioctl functions
 *
 * This program is NOT-free software; You can redistribute it
 * and/or modify it under the terms of the Albam's General Public
 * License (AGPL) as published by the albam.
 *
 * Albam's General Public License (AGPL)
 * DO NOT USE OR MODIFY MY SOURCES!!!
 *
 * Changes :
 *  - 2005/12/05 albam <albamc@gmail.com> : make initial version.
 */

#include "ioctl_func.h"

int
sioc_addrt(uint32_t dst, uint32_t mask, uint32_t gw, char* dev)
{
	int fd, ret;
	struct rtentry rt;
	
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&rt, 0, sizeof(struct rtentry));
	_saddr((struct sockaddr_in*)&rt.rt_dst, dst, 0);
	_saddr((struct sockaddr_in*)&rt.rt_genmask, mask, 0);
	if (gw) {
		_saddr((struct sockaddr_in*)&rt.rt_gateway, gw, 0);
		rt.rt_flags |= RTF_GATEWAY;
	}
	if (dev)
		rt.rt_dev = dev;
	rt.rt_flags |= RTF_UP;
	if (gw)
		rt.rt_flags |= RTF_GATEWAY;
	ret = ioctl(fd, SIOCADDRT, &rt);
	close(fd);

	return ret;
}

int
sioc_delrt(uint32_t dst, uint32_t mask, uint32_t gw, char* dev)
{
	int fd, ret;
	struct rtentry rt;
	
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&rt, 0, sizeof(struct rtentry));
	_saddr((struct sockaddr_in*)&rt.rt_dst, dst, 0);
	_saddr((struct sockaddr_in*)&rt.rt_genmask, mask, 0);
	_saddr((struct sockaddr_in*)&rt.rt_gateway, gw, 0);
	if (dev)
		rt.rt_dev = dev;
	ret = ioctl(fd, SIOCDELRT, &rt);	
	close(fd);

	return ret;
}

/* skip SIOCRTMSG */

int
sioc_gifname(int ifindex, char* name)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	ifr.ifr_ifindex = ifindex;
	ret = ioctl(fd, SIOCGIFNAME, &ifr);	
	if (ret == 0)
		strcpy(name, ifr.ifr_name);
	close(fd);

	return ret;
}

/* skip SIOCSIFLINK */

/* no test */
int
sioc_gifconf(char* buf, int len)
{
	int fd, ret;
	struct ifconf ifc;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifc, 0, sizeof(struct ifconf));
	ifc.ifc_len = len;
	ifc.ifc_buf = buf;
	ret = ioctl(fd, SIOCGIFCONF, &ifc);	
	close(fd);

	return ret;
}

uint16_t 
sioc_gifflags(char* name)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	ret = ioctl(fd, SIOCGIFFLAGS, &ifr);	
	close(fd);

	return ifr.ifr_flags;
}

int
sioc_sifflags(char* name, uint16_t flags)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	ifr.ifr_flags = flags;
	ret = ioctl(fd, SIOCSIFFLAGS, &ifr);	
	close(fd);

	return ret;
}

uint32_t
sioc_gifaddr(char* name)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	ret = ioctl(fd, SIOCGIFADDR, &ifr);	
	close(fd);

	return (((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr.s_addr);
}

int
sioc_sifaddr(char* name, uint32_t addr)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	_saddr((struct sockaddr_in*)&ifr.ifr_addr, addr, 0);
	ret = ioctl(fd, SIOCSIFADDR, &ifr);	
	close(fd);

	return ret;
}

uint32_t
sioc_gifdstaddr(char* name)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	ret = ioctl(fd, SIOCGIFDSTADDR, &ifr);	
	close(fd);

	return (((struct sockaddr_in*)&ifr.ifr_dstaddr)->sin_addr.s_addr);
}

int
sioc_sifdstaddr(char* name, uint32_t addr)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	_saddr((struct sockaddr_in*)&ifr.ifr_dstaddr, addr, 0);
	ret = ioctl(fd, SIOCSIFDSTADDR, &ifr);	
	close(fd);

	return ret;
}

uint32_t
sioc_gifbrdaddr(char* name)
{
	int fd, ret;
	struct ifreq ifr;
	
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	ret = ioctl(fd, SIOCGIFBRDADDR, &ifr);
	close(fd);

	return (((struct sockaddr_in*)&ifr.ifr_broadaddr)->sin_addr.s_addr);
}

int
sioc_sifbrdaddr(char* name, uint32_t addr)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	_saddr((struct sockaddr_in*)&ifr.ifr_broadaddr, addr, 0);
	ret = ioctl(fd, SIOCSIFBRDADDR, &ifr);	
	close(fd);

	return ret;
}

uint32_t
sioc_gifnetmask(char* name)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	ret = ioctl(fd, SIOCGIFNETMASK, &ifr);	
	close(fd);

	return (((struct sockaddr_in*)&ifr.ifr_netmask)->sin_addr.s_addr);
}

int
sioc_sifnetmask(char* name, uint32_t mask)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	_saddr((struct sockaddr_in*)&ifr.ifr_netmask, mask, 0);
	ret = ioctl(fd, SIOCSIFNETMASK, &ifr);	
	close(fd);

	return ret;
}

/* skip SIOCSIFMETRIC */
/* skip SIOCGIFMETRIC */
/* skip SIOCGIFMEM */
/* skip SIOCSIFMEM */

int
sioc_gifmtu(char* name)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	ret = ioctl(fd, SIOCGIFMTU, &ifr);	
	close(fd);

	return ifr.ifr_mtu;
}

int
sioc_sifmtu(char* name, int mtu)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	ifr.ifr_mtu = mtu;
	ret = ioctl(fd, SIOCSIFMTU, &ifr);	
	close(fd);

	return ret;
}

/* only allowed interface not up */
int
sioc_sifname(char* org_name, char* new_name)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, org_name, IFNAMSIZ);
	strncpy(ifr.ifr_newname, new_name, IFNAMSIZ);
	ret = ioctl(fd, SIOCSIFNAME, &ifr);	
	close(fd);

	return ret;
}

int
sioc_sifhwaddr(char* name, char* addr)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	_shwaddr(&ifr.ifr_hwaddr, addr);
	ret = ioctl(fd, SIOCSIFHWADDR, &ifr);	
	close(fd);

	return ret;
}

/* skip SIOCGIFENCAP */
/* skip SIOCSIFENCAP */

int
sioc_gifhwaddr(char* name, char* addr)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	ret = ioctl(fd, SIOCGIFHWADDR, &ifr);	
	memcpy(addr, (char*)ifr.ifr_hwaddr.sa_data, IFHWADDRLEN);
	close(fd);

	return ret;
}

/* skip SIOCGIFSLAVE */
/* skip SIOCSIFSLAVE (deprecated SIOCBONDENSLAVE) */
/* skip SIOCADDMULTI */
/* skip SIOCDELMULTI */

int
sioc_gifindex(char* name)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	ret = ioctl(fd, SIOCGIFINDEX, &ifr);	
	close(fd);

	return ifr.ifr_ifindex;
}

/* skip SIOCSIFPFLAGS */
/* skip SIOCGIFPFLAGS */
/* skip SIOCDIFADDR */

int
sioc_sifhwbroadcast(char* name, char* addr)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	_shwaddr(&ifr.ifr_hwaddr, addr);
	ret = ioctl(fd, SIOCSIFHWBROADCAST, &ifr);	
	close(fd);

	return ret;
}

/* skip SIOCGIFCOUNT */
/* skip SIOCGIFBR */
/* skip SIOCSIFBR */

int
sioc_giftxqlen(char* name)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	ret = ioctl(fd, SIOCGIFTXQLEN, &ifr);	
	close(fd);

	return ifr.ifr_qlen;
}

int
sioc_siftxqlen(char* name, int qlen)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	ifr.ifr_qlen = qlen;
	ret = ioctl(fd, SIOCSIFTXQLEN, &ifr);	
	close(fd);

	return ret;
}

/* skip SIOCGIFDIVERT */
/* skip SIOCSIFDIVERT */

int
sioc_ethtool(char* name, struct ethtool_drvinfo* info
				, uint32_t cmd, char* driver)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	ifr.ifr_data = (caddr_t)info;
	info->cmd = cmd;
	if (driver)
		strncpy(info->driver, driver, 32);
	ret = ioctl(fd, SIOCETHTOOL, &ifr);	
	close(fd);

	return ret;
}

int
sioc_gmiiphy(char* name, struct mii_ioctl_data* data)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	ifr.ifr_data = (caddr_t)data;
	ret = ioctl(fd, SIOCGMIIPHY, &ifr);	
	close(fd);

	return ret;
}

int
sioc_gmiireg(char* name, struct mii_ioctl_data* data)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	ifr.ifr_data = (caddr_t)data;
	ret = ioctl(fd, SIOCGMIIREG, &ifr);	
	close(fd);

	return ret;
}

int
sioc_smiireg(char* name, struct mii_ioctl_data* data)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	ifr.ifr_data = (caddr_t)data;
	ret = ioctl(fd, SIOCSMIIREG, &ifr);	
	close(fd);

	return ret;
}

/* skip SIOCWANDEV */

int
sioc_darp(char* name, uint32_t paddr, char* hwaddr, uint32_t mask)
{
	int fd, ret;
	struct arpreq ar;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ar, 0, sizeof(struct arpreq));
	_saddr((struct sockaddr_in*)&ar.arp_pa, paddr, 0);
	_shwaddr(&ar.arp_ha, hwaddr);
	if (mask)
		_saddr((struct sockaddr_in*)&ar.arp_netmask, mask, 0);
	strncpy(ar.arp_dev, name, IFNAMSIZ);
	ret = ioctl(fd, SIOCDARP, &ar);	
	close(fd);

	return ret;
}

int
sioc_garp(char* name, uint32_t paddr, char* hwaddr)
{
	int fd, ret;
	struct arpreq ar;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ar, 0, sizeof(struct arpreq));
	_saddr((struct sockaddr_in*)&ar.arp_pa, paddr, 0);
	strncpy(ar.arp_dev, name, IFNAMSIZ);
	ret = ioctl(fd, SIOCGARP, &ar);	
	memcpy(hwaddr, ar.arp_ha.sa_data, IFHWADDRLEN);
	close(fd);

	return ret;
}

int
sioc_sarp(char* name, uint32_t paddr, char* hwaddr, int flags, uint32_t mask)
{
	int fd, ret;
	struct arpreq ar;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ar, 0, sizeof(struct arpreq));
	_saddr((struct sockaddr_in*)&ar.arp_pa, paddr, 0);
	_shwaddr(&ar.arp_ha, hwaddr);
	ar.arp_flags = (ATF_PERM | ATF_COM | flags);
	if (mask) {
		ar.arp_flags |= ATF_NETMASK;
		_saddr((struct sockaddr_in*)&ar.arp_netmask, mask, 0);
	}
	strncpy(ar.arp_dev, name, IFNAMSIZ);
	ret = ioctl(fd, SIOCSARP, &ar);	
	close(fd);

	return ret;
}

/* skip SIOCDRARP */
/* skip SIOCGRARP */
/* skip SIOCDRARP */
/* skip SIOCGIFMAP */
/* skip SIOCSIFMAP */
/* skip SIOCADDDLCI */
/* skip SIOCDELDLCI */
/* skip SIOCGIFVLAN */
/* skip SIOCSIFVLAN */

int
sioc_bondenslave(char* master, char* slave)
{
	int fd, ret;
	struct ifreq ifr;
	uint16_t flags;
	char hwaddr[IFHWADDRLEN], thwaddr[IFHWADDRLEN];

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;

	/* Ckeck for slave interface already a slave */
	flags = sioc_gifflags(slave);
	if (flags & IFF_SLAVE) 
		return -1;

	/* Set slave interface down */
	flags &= ~IFF_UP;
	sioc_sifflags(slave, flags);
	
	/* Set slave interface IP address */
	ret = sioc_sifaddr(slave, 0); 
	if (ret < 0)
		return -1;

	/* Set slave interface mtu */
	ret = sioc_sifmtu(slave, sioc_gifmtu(master));
	if (ret < 0)
		return -1;
	
	/* Check Master interface hwaddr */
	ret = sioc_gifhwaddr(master, hwaddr);
	if (ret < 0)
		return -1;
	memset(thwaddr, 0, IFHWADDRLEN);
	if (memcmp(hwaddr, thwaddr, IFHWADDRLEN) == 0) {
		ret = sioc_gifhwaddr(slave, hwaddr);
		if (ret < 0)
			return -1;
		ret = sioc_sifhwaddr(master, hwaddr);
		if (ret < 0)
			return -1;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, master, IFNAMSIZ);
	strncpy(ifr.ifr_slave, slave, IFNAMSIZ);
	ret = ioctl(fd, SIOCBONDENSLAVE, &ifr);	
	if (ret < 0)
		ret = ioctl(fd, BOND_ENSLAVE_OLD, &ifr);
	close(fd);

	return ret;
}

int
sioc_bondrelease(char* master, char* slave)
{
	int fd, ret;
	struct ifreq ifr;
	uint16_t flags;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;

	/* Ckeck for slave interface already a slave */
	flags = sioc_gifflags(slave);
	if (!(flags & IFF_SLAVE)) 
		return -1;

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, master, IFNAMSIZ);
	strncpy(ifr.ifr_slave, slave, IFNAMSIZ);
	ret = ioctl(fd, SIOCBONDRELEASE, &ifr);	
	if (ret < 0)
		ret = ioctl(fd, BOND_RELEASE_OLD, &ifr);
	close(fd);

	return ret;
}

/* set master's hwaddr to slave */
int
sioc_bondsethwaddr(char* master, char* slave)
{
	int fd, ret;
	struct ifreq ifr;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, master, IFNAMSIZ);
	strncpy(ifr.ifr_slave, slave, IFNAMSIZ);
	ret = ioctl(fd, SIOCBONDSETHWADDR, &ifr);	
	if (ret < 0)
		ret = ioctl(fd, BOND_SETHWADDR_OLD, &ifr);
	close(fd);

	return ret;
}

/* skip SIOCBONDSLAVEINFOQUERY */
/* skip SIOCBONDINFOQUERY */

int
sioc_bondchangeactive(char* master, char* slave)
{
	int fd, ret;
	struct ifreq ifr;
	uint16_t flags;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;

	/* Ckeck for slave interface already a slave */
	flags = sioc_gifflags(slave);
	if (!(flags & IFF_SLAVE)) 
		return -1;

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, master, IFNAMSIZ);
	strncpy(ifr.ifr_slave, slave, IFNAMSIZ);
	ret = ioctl(fd, SIOCBONDCHANGEACTIVE, &ifr);	
	if (ret < 0)
		ret = ioctl(fd, BOND_CHANGE_ACTIVE_OLD, &ifr);
	close(fd);

	return ret;
}

/* skip SIOCBRADDBR */
/* skip SIOCBRDELBR */
/* skip SIOCBRADDIF */
/* skip SIOCBRDELIF */

int
sioc_devprivate(char* dev, uint8_t* p, int n)
{
	int fd, ret;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(p, 0, n);
	ret = ioctl(fd, SIOCDEVPRIVATE, p);	
	close(fd);

	return ret;
}

/* skip SIOCPROTOPRIVATE*/

//define TESTCODE
#ifdef TESTCODE

#include <stdlib.h>
#include <arpa/inet.h>

#define DEV "eth4"
#define DEST "1.1.1.1"
#define MASK "255.255.255.255"
#define GW "10.120.1.254"
#define IFINDEX 1
#define HWADDR "000000"

int
main(void)
{
	uint32_t dest, mask, gw, addr;
	char name[16], hwaddr[6];
	uint16_t flags;
	int ifindex;

	dest = inet_addr(DEST);
	mask = inet_addr(MASK);
	gw = inet_addr(GW);

	printf("route add -net %s netmask %s dev %s gw %s\n", DEST, MASK, DEV, GW);
	sioc_addrt(dest, mask, gw, DEV);
	system("route");

	printf("route delete -net %s netmask %s dev %s gw %s\n", DEST, MASK, DEV, GW);
	sioc_delrt(dest, mask, gw, DEV);
	system("route");

	sioc_gifname(IFINDEX, name);
	printf("INDEX:%d NAME:%s\n", IFINDEX, name);

	flags = sioc_gifflags(DEV);
	printf("Device %s Flags\n", DEV);
	printf("IFF_UP         :%s\n", (flags & IFF_UP)?"yes":"no");
	printf("IFF_BROADCAST  :%s\n", (flags & IFF_BROADCAST)?"yes":"no");
	printf("IFF_DEBUG      :%s\n", (flags & IFF_DEBUG)?"yes":"no");
	printf("IFF_LOOPBACK   :%s\n", (flags & IFF_LOOPBACK)?"yes":"no");
	printf("IFF_POINTOPOINT:%s\n", (flags & IFF_POINTOPOINT)?"yes":"no");
	printf("IFF_NOTRAILERS :%s\n", (flags & IFF_NOTRAILERS)?"yes":"no");
	printf("IFF_RUNNING    :%s\n", (flags & IFF_RUNNING)?"yes":"no");
	printf("IFF_NOARP      :%s\n", (flags & IFF_NOARP)?"yes":"no");
	printf("IFF_PROMISC    :%s\n", (flags & IFF_PROMISC)?"yes":"no");
	printf("IFF_ALLMULTI   :%s\n", (flags & IFF_ALLMULTI)?"yes":"no");
	printf("IFF_MASTER     :%s\n", (flags & IFF_MASTER)?"yes":"no");
	printf("IFF_SLAVE      :%s\n", (flags & IFF_SLAVE)?"yes":"no");
	printf("IFF_MULTICAST  :%s\n", (flags & IFF_MULTICAST)?"yes":"no");

	printf("Set %s to Promisc mode\n", DEV);
	flags |= IFF_PROMISC;
	sioc_sifflags(DEV, flags);

	flags = sioc_gifflags(DEV);
	printf("Device %s Flags\n", DEV);
	printf("IFF_UP         :%s\n", (flags & IFF_UP)?"yes":"no");
	printf("IFF_BROADCAST  :%s\n", (flags & IFF_BROADCAST)?"yes":"no");
	printf("IFF_DEBUG      :%s\n", (flags & IFF_DEBUG)?"yes":"no");
	printf("IFF_LOOPBACK   :%s\n", (flags & IFF_LOOPBACK)?"yes":"no");
	printf("IFF_POINTOPOINT:%s\n", (flags & IFF_POINTOPOINT)?"yes":"no");
	printf("IFF_NOTRAILERS :%s\n", (flags & IFF_NOTRAILERS)?"yes":"no");
	printf("IFF_RUNNING    :%s\n", (flags & IFF_RUNNING)?"yes":"no");
	printf("IFF_NOARP      :%s\n", (flags & IFF_NOARP)?"yes":"no");
	printf("IFF_PROMISC    :%s\n", (flags & IFF_PROMISC)?"yes":"no");
	printf("IFF_ALLMULTI   :%s\n", (flags & IFF_ALLMULTI)?"yes":"no");
	printf("IFF_MASTER     :%s\n", (flags & IFF_MASTER)?"yes":"no");
	printf("IFF_SLAVE      :%s\n", (flags & IFF_SLAVE)?"yes":"no");
	printf("IFF_MULTICAST  :%s\n", (flags & IFF_MULTICAST)?"yes":"no");

	printf("Unset %s to Promisc mode\n", DEV);
	flags &= ~IFF_PROMISC;
	sioc_sifflags(DEV, flags);

	addr = inet_addr("18.18.18.18");
	sioc_sifaddr(DEV, addr);

	addr = sioc_gifaddr(DEV);
	printf("Dev %s IP address:%d.%d.%d.%d (host order)\n", DEV
			, ((uint8_t*)&addr)[0]
			, ((uint8_t*)&addr)[1]
			, ((uint8_t*)&addr)[2]
			, ((uint8_t*)&addr)[3]);
	system("ifconfig eth4");

	addr = sioc_gifdstaddr(DEV);
	printf("Dev %s DST IP address:%d.%d.%d.%d (host order)\n", DEV
			, ((uint8_t*)&addr)[0]
			, ((uint8_t*)&addr)[1]
			, ((uint8_t*)&addr)[2]
			, ((uint8_t*)&addr)[3]);

	sioc_sifdstaddr(DEV, addr);

	addr = sioc_gifdstaddr(DEV);
	printf("Dev %s DST IP address:%d.%d.%d.%d (host order)\n", DEV
			, ((uint8_t*)&addr)[0]
			, ((uint8_t*)&addr)[1]
			, ((uint8_t*)&addr)[2]
			, ((uint8_t*)&addr)[3]);

	addr = sioc_gifbrdaddr(DEV);
	printf("Dev %s Broadcast IP address:%d.%d.%d.%d (host order)\n", DEV
			, ((uint8_t*)&addr)[0]
			, ((uint8_t*)&addr)[1]
			, ((uint8_t*)&addr)[2]
			, ((uint8_t*)&addr)[3]);
	
	addr = inet_addr("18.18.255.255");
	printf("Set broadcast address 18.18.255.255\n");
	sioc_sifbrdaddr(DEV, addr);
	system("ifconfig eth4");

	addr = sioc_gifnetmask(DEV);
	printf("Get Netmask %d.%d.%d.%d\n"
			, ((uint8_t*)&addr)[0]
			, ((uint8_t*)&addr)[1]
			, ((uint8_t*)&addr)[2]
			, ((uint8_t*)&addr)[3]);

	addr = inet_addr("255.255.0.0");
	printf("set netmask 255.255.0.0\n");
	sioc_sifnetmask(DEV, addr);

	addr = sioc_gifnetmask(DEV);
	printf("Get Netmask %d.%d.%d.%d\n"
			, ((uint8_t*)&addr)[0]
			, ((uint8_t*)&addr)[1]
			, ((uint8_t*)&addr)[2]
			, ((uint8_t*)&addr)[3]);

	printf("Set MTU to 1024 (R:%d)\n", sioc_sifmtu(DEV, 1024));
	printf("Get MTU : %d\n", sioc_gifmtu(DEV));
	system("ifconfig eth4");
	printf("Set MTU to 1500 (R:%d)\n", sioc_sifmtu(DEV, 1500));

	ifindex = sioc_gifindex(DEV);
	printf("DEV %s ifindex= %d\n", DEV, ifindex);
	sioc_gifname(ifindex, name);
	printf("ifindex %d device name is %s\n", ifindex, name);
	printf("name change ret %d\n", sioc_sifname(name, "eth18"));
	sioc_gifname(ifindex, name);
	printf("ifindex %d device name chaned to %s\n", ifindex, name);
	sioc_sifname("eth18", DEV);

	sioc_gifhwaddr(DEV, hwaddr);
	printf("%s hwaddr %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n"
		, DEV
		, hwaddr[0]
		, hwaddr[1]
		, hwaddr[2]
		, hwaddr[3]
		, hwaddr[4]
		, hwaddr[5]
		);
	memset(hwaddr, 0, IFHWADDRLEN);
	hwaddr[5] = 0x18;
	printf("dev %s Set HW result %d\n", DEV, sioc_sifhwaddr(DEV, hwaddr));
	sioc_gifhwaddr(DEV, hwaddr);
	printf("hwaddr set to %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n"
		, hwaddr[0]
		, hwaddr[1]
		, hwaddr[2]
		, hwaddr[3]
		, hwaddr[4]
		, hwaddr[5]
		);

	printf("ARPSET (R:%d)\n"
		, sioc_sarp(DEV, inet_addr("10.120.1.218"), hwaddr, 0, 0));

	memset(hwaddr, 0, IFHWADDRLEN);
	sioc_garp(DEV, inet_addr("10.120.1.218"), hwaddr);
	printf("ipaddr 10.120.1.218 HW: %x:%x:%x:%x:%x:%x\n"
		, hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
	sioc_darp(DEV, inet_addr("10.120.1.218"), hwaddr, 0);
	system("arp");
	printf("ENSLAVE RESULT:%d\n", sioc_bondenslave("bond1", "eth4"));
	system("ifconfig");
	printf("RELEASE SLAVE:%d\n", sioc_bondrelease("bond1", "eth4"));
	system("ifconfig");

	return 0;
}

#endif

