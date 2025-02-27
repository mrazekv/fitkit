all: bmp2rif bmp2raf avi2raf
bmp2rif:
	gcc -O2 bmp2rif.c -o bmp2rif

bmp2raf:
	gcc -O2 bmp2raf.c -o bmp2raf

CFLAGS=-O2 -ansi -Wall -Iinclude
LDFLAGS=-Llib -lvfw32

avi2raf:
	gcc -o avi2raf.exe $(CFLAGS) avi2raf.cpp $(LDFLAGS)
