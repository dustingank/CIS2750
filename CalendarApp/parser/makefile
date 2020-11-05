all: list parser library

list: src/LinkedListAPI.c include/LinkedListAPI.h
	gcc -fPIC -Iinclude -Wall -std=c11 -g -c src/LinkedListAPI.c -o liblist.o

parser: src/CalendarParser.c include/CalendarParser.h
	gcc -fPIC -Iinclude -Wall -std=c11 -g -c src/CalendarParser.c -o libcal.o

library: liblist.o libcal.o
	gcc liblist.o libcal.o -shared -o ../library.so
