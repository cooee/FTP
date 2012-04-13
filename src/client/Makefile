CC=gcc
CFLAGS=-Wall
FDLAGS=-lpthread

all:ftp

ftp: main.o ftp.o
	$(CC) $(CFLAGS) $^ -o $@

main.o ftp.o: ftp.h

clean:
	@rm -frv *.o ftp

