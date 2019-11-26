OUT=mmi
SRC=main.c mmc.c
OBJ=$(SRC:.c=.o)

CC=gcc
CFLAGS=-g -I. -I./lib -I./json-c -I/usr/local/include
LDFLAGS=-L./lib -lpopup -ljson-c -lpthread



all: $(OUT)

$(OUT): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)
	cp $@ ../bin 

clean:
	rm -rf $(OUT) $(OBJ)

