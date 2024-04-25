PKGS=raylib

CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb `pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --libs $(PKGS)` -lm

main: main.o
	$(CC) $(CFLAGS) -o zoomer main.o $(LIBS)

main.o: main.c
	$(CC) $(CFLAGS) -o main.o -c main.c
