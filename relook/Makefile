CC=gcc
LD=ld
CFLAGS=-Wall -fPIC -DPIC
LDFLAGS=-shared -ldl
all:
	$(CC) $(CFLAGS) -c relook.c
	$(LD) $(LDFLAGS) -o relook.so relook.o
