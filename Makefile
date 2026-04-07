CC = gcc
CFLAGS = -Wall -g
LIBS = -lreadline -lpthread -lm

all: imesh ep1

imesh: imesh.c imesh.h
	$(CC) $(CFLAGS) imesh.c -o imesh $(LIBS)

ep1: ep1.c ep1.h
	$(CC) $(CFLAGS) ep1.c -o ep1 $(LIBS)

clean:
	rm -f imesh ep1 *.o