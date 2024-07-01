PKGS=raylib x11 xcursor

CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb `pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --libs $(PKGS)` -lm
INSTALL_DIR?=/usr/local/bin

zoomer: main.o
	$(CC) $(CFLAGS) -o zoomer main.o $(LIBS)

main.o: main.c
	$(CC) $(CFLAGS) -o main.o -c main.c

install: zoomer
	install -m 755 zoomer $(INSTALL_DIR)

uninstall:
	rm -f $(INSTALL_DIR)/zoomer
