CC	= gcc
INC 	= -I..
LIBS = -L.. -lmnl -lpthread
CFLAGS 	= -ggdb $(INC) -DLinux

SRCS = \
mmc.c                 test_cbtimer.c  test_imsi.c          test_mem.c     test_tcpcli.c \
premmc.c              test_conf.c     test_interface.c     test_mmc.c     test_tcpsvr.c \
progA.c               test_cpu.c      test_ip.c            test_msgq.c    test_time.c \
progB.c               test_csum.c     test_ippool.c        test_msleep.c  test_timer.c \
test_access.c         test_dhcp.c     test_log.c           test_ntp.c     test_udp.c \
test_account.c        test_disk.c     test_mac_parse.c     test_peer.c    test_v6_tcpclnt.c \
test_base64.c         test_dns.c      test_make_access.c   test_ping.c    test_v6_tcpsvr.c \
test_bcd.c            test_dump.c     test_make_account.c  test_shell.c \
test_bg.c             test_icmp.c     test_make_dhcp.c     test_shm.c \
test_cbtimer_block.c  test_if.c       test_make_rrq.c      test_sleep.c

OBJS = $(SRCS:.c=.o)

TARGET 	= test

all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o test_access test_access.o $(LIBS)
	$(CC) $(CFLAGS) -o test_conf test_conf.o $(LIBS)
	$(CC) $(CFLAGS) -o test_icmp test_icmp.o  $(LIBS)
	$(CC) $(CFLAGS) -o test_mac_parse test_mac_parse.o $(LIBS)
	$(CC) $(CFLAGS) -o test_msgq test_msgq.o $(LIBS)
	$(CC) $(CFLAGS) -o test_sleep test_sleep.o $(LIBS)
	$(CC) $(CFLAGS) -o test_v6_tcpsvr test_v6_tcpsvr.o $(LIBS)
	$(CC) $(CFLAGS) -o test_account test_account.o $(LIBS)
	$(CC) $(CFLAGS) -o test_cpu test_cpu.o $(LIBS)
#	$(CC) $(CFLAGS) -o test_if test_if.o $(LIBS)
	$(CC) $(CFLAGS) -o test_make_access test_make_access.o $(LIBS)
	$(CC) $(CFLAGS) -o test_msleep test_msleep.o $(LIBS)
	$(CC) $(CFLAGS) -o test_tcpcli test_tcpcli.o $(LIBS)
	$(CC) $(CFLAGS) -o test_base64 test_base64.o $(LIBS)
	$(CC) $(CFLAGS) -o test_csum test_csum.o $(LIBS)
	$(CC) $(CFLAGS) -o test_imsi test_imsi.o $(LIBS)
	$(CC) $(CFLAGS) -o test_make_account test_make_account.o $(LIBS)
#	$(CC) $(CFLAGS) -o test_ntp test_ntp.o $(LIBS)
	$(CC) $(CFLAGS) -o test_tcpsvr test_tcpsvr.o $(LIBS)
	$(CC) $(CFLAGS) -o test_bcd test_bcd.o $(LIBS)
	$(CC) $(CFLAGS) -o test_dhcp test_dhcp.o $(LIBS)
	$(CC) $(CFLAGS) -o test_interface test_interface.o $(LIBS)
	$(CC) $(CFLAGS) -o test_make_dhcp test_make_dhcp.o $(LIBS)
	$(CC) $(CFLAGS) -o test_peer test_peer.o $(LIBS)
	$(CC) $(CFLAGS) -o test_time test_time.o $(LIBS)
	$(CC) $(CFLAGS) -o test_bg test_bg.o $(LIBS)
	$(CC) $(CFLAGS) -o test_disk test_disk.o $(LIBS)
	$(CC) $(CFLAGS) -o test_ip test_ip.o $(LIBS)
	$(CC) $(CFLAGS) -o test_make_rrq test_make_rrq.o $(LIBS)
	$(CC) $(CFLAGS) -o test_ping test_ping.o $(LIBS)
	$(CC) $(CFLAGS) -o test_timer test_timer.o $(LIBS)
	$(CC) $(CFLAGS) -o test_cbtimer_block test_cbtimer_block.o $(LIBS)
#	$(CC) $(CFLAGS) -o test_dns test_dns.o $(LIBS)
	$(CC) $(CFLAGS) -o test_ippool test_ippool.o $(LIBS)
	$(CC) $(CFLAGS) -o test_mem test_mem.o $(LIBS)
	$(CC) $(CFLAGS) -o test_shell test_shell.o mmc.o premmc.o $(LIBS)
	$(CC) $(CFLAGS) -o test_udp test_udp.o $(LIBS)
	$(CC) $(CFLAGS) -o test_cbtimer test_cbtimer.o $(LIBS)
	$(CC) $(CFLAGS) -o test_dump test_dump.o $(LIBS)
	$(CC) $(CFLAGS) -o test_log test_log.o $(LIBS)
	$(CC) $(CFLAGS) -o test_mmc test_mmc.o mmc.o premmc.o $(LIBS)
	$(CC) $(CFLAGS) -o test_shm test_shm.o $(LIBS)
	$(CC) $(CFLAGS) -o test_v6_tcpclnt test_v6_tcpclnt.o $(LIBS)

dep:
	gccmakedep $(INC) $(SRCS)

clean:
	rm -rf $(OBJS) $(TARGET) core
