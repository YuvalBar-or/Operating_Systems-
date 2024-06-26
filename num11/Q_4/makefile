CC = gcc
CFLAGS = -Wall

LIBS = -lc

all : myunzip myzip

myzip : myzip.o
	$(CC) $(CFLAGS) myzip.o -o myzip

myunzip : myunzip.o
	$(CC) $(CFLAGS) myunzip.o -o myunzip

.PHONY : all clean

clean :
	rm *.o myzip myunzip