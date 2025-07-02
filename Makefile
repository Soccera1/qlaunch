PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

CC = cc
CFLAGS = -std=c99 -Wall -Wextra -Werror -pedantic -O3 -march=native
LDFLAGS = -lX11

SRC = qlaunch.c
OBJ = $(SRC:.c=.o)
TARGET = qlaunch

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(LDFLAGS)

%.o: %.c config.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(TARGET) $(OBJ)

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(TARGET) $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/$(TARGET)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)

.PHONY: all clean install uninstall
