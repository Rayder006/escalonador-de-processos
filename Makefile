CC = gcc
CFLAGS = -Wall -g

all: imesh ep1

imesh: imesh.c imesh.h
	$(CC) $(CFLAGS) imesh.c

ep1: ep1.c ep1.h
	$(CC) $(CFLAGS) ep1.c -o ep1

clean:
	rm -f imesh ep1