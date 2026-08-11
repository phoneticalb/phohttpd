GIT_VERSION := "$(shell git describe --abbrev=0 --tags)"
CC = gcc
CFLAGS = -Wall -DVERSION=\"$(GIT_VERSION)\"
LDFLAGS =
BINARY = ./phohttpd
SOURCES = ./src/http.c ./src/server.c ./src/main.c

all: $(BINARY)

debug: CFLAGS += -g
debug: $(BINARY)

$(BINARY): $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SOURCES)

clear:
	rm -f ./phohttpd

.PHONY: all debug clear
