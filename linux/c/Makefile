CFLAGS=-D__LINUX__ -Wall -O2 -I. -g
LIBS =

all: jpegenc

jpegenc: main.o jpegenc.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

clean:
	rm -rf *.o jpegenc
