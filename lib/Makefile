CC	= gcc
INC 	= 
LIBS 	=
#CFLAGS 	= -ggdb $(INC) -fPIC -DLinux
CFLAGS 	= -ggdb $(INC) -DLinux

SRCS = \
bufq.c disk.c   hmac_md5.c  ioctl_func.c log.c         make_rrq.c  proc.c   shell.c  \
conf.c ip.c     mac_parse.c if.c         md5.c         peer.c      sock.c  \
cpu.c  dump.c   imsi.c      ntp.c        ipc.c         make_dhcp.c mem.c    ping.c   sockv6.c \
dhcp.c encode.c interface.c ipp.c        make_radius.c mmc.c       premmc.c radius.c time.c timer.c seed.c base64.c

# if.c ntp.c dns.c shell.c

OBJS = $(SRCS:.c=.o)
LIBS = -L. -lmnl -lpthread

#TARGET 	= libmnl.so.1 libmnl.a
TARGET 	= libpopup.a

all: $(TARGET)

libpopup.a : $(OBJS)
	$(AR) -rcv $@ $(OBJS)
	ranlib $@

libmnl.so.1 : $(OBJS)
	$(CC) -shared -Wl,-soname,$@ -o $@ $(OBJS)

dep:
	gccmakedep $(INC) $(SRCS)

clean:
	rm -rf $(OBJS) $(TARGET) core
