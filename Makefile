CC = gcc
CFLAGS = -Wall -g
LIBS = -lpthread -lreadline -lm

all: imesh ep1

imesh: imesh.c imesh.h
	$(CC) $(CFLAGS) imesh.c $(LIBS)

ep1: ep1.c ep1.h
	$(CC) $(CFLAGS) ep1.c -o ep1 $(LIBS)

clean:
	rm -f ep1 imesh *.o