# makefile for distributed project 1

all: rm
	
rm: rm.c utils.o
	gcc rm.c utils.o -o rm

dv: dv.c utils.o
	gcc dv.c utils.o -o dv

DUMP: dump.c utils.o
	gcc dump.c utils.o -o DUMP

utils.o: utils.c utils.h
	gcc utils.c -c -o utils.o

clean:
	rm -f -r rm dv utils.o mytest* DUMP