CC = gcc
CFLAGS = -g3 -Wall

all: mains

mains: main.o bugs.o bugs.h
	$(CC) $(CFLAGS) main.o bugs.o -o mains

.PHONY: all clean

clean:
	rm mains *.o