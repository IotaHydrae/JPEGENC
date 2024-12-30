CFLAGS=-D__LINUX__ -Wall -O2 -g
LIBS =

all: jpegenc

jpegenc: main.o
	$(CC) main.o  $(LIBS) -o jpegenc

main.o: main.c
	$(CC) $(CFLAGS) -c main.c jpegenc.c JPEGENC.h

clean:
	rm -rf *.o jpegenc
