CC = gcc
CFLAGS = -Wall

all : libencriptor.so encode decode

libencriptor.so : funcs.c funcs.h
	$(CC) $(CFLAGS) -fPIC -c funcs.c
	$(CC) $(CFLAGS) -shared funcs.o -o libencriptor.so

encode : funcs.o funcs.h encode.o coding.o coding.h
	$(CC) $(CFLAGS) funcs.o encode.o coding.o -o encode

decode : funcs.o funcs.h decode.o coding.o coding.h
	$(CC) $(CFLAGS) funcs.o decode.o coding.o -o decode

.PHONY : all clean

clean:
	rm encode decode *.so *.o