OUT=mmi
SRC=main.c mmc.c
OBJ=$(SRC:.c=.o)

CC=gcc
CFLAGS=-g -w -DLinux -I. -I./lib -I./json-c -I/usr/local/include 
LDFLAGS=-L./lib -lpopup -ljson-c -lpthread

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

clean:
	rm -rf $(OUT) $(OBJ) dist/*.deb

install:
	cp $(OUT) ../../DEBIAN/utils/radix-utils/opt/radix/bin
	cp *.a ../../DEBIAN/utils/radix-utils/opt/radix/lib/
	cp -r scripts ../../DEBIAN/utils/radix-utils/opt/radix/

