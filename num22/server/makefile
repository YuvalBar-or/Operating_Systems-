
CC = gcc
CFLAGS = -Wall


all: server basicClient

.PHONY: clean all


server: server.o
	$(CC) $(CFLAGS) -o server server.o
basicClient: basicClient.o
	$(CC) $(CFLAGS) -o basicClient basicClient.o 



clean:
	rm -f server basicClient *.o
