CC = gcc
CFLAGS = -Wall -g
LIBS = -lssl -lcrypto

all: client

.PHONY: clean all

client: client.o utils.o client.h utils.h
	$(CC) $(CFLAGS) -o client client.o utils.o $(LIBS)

clean:
	rm -f client *.o