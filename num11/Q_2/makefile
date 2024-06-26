CC = gcc
CFLAGS = -Wall

LIBS = -lm

all : pifagor3

pifagor3 : functions.h functions.o main.o
	$(CC) $(CFLAGS) main.o functions.o -o pifagor3 $(LIBS)

.PHONY : all clean

clean:
	rm pifagor3 *.o