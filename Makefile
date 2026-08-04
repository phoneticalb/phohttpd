GIT_VERSION := "$(shell git describe --abbrev=0 --tags)"
CC = gcc
CFLAGS = -Wall -DVERSION=\"$(GIT_VERSION)\"
LDFLAGS =
BINARY = ./build/phohttpd
SOURCES = ./src/http.c ./src/server.c ./src/main.c
INCLUDES = ./build

all: $(BINARY)

debug: CFLAGS += -g
debug: $(BINARY)

$(BINARY): $(SOURCES)
	mkdir -p ./build
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SOURCES) -I $(INCLUDES)

clear:
	rm -rf ./build

.PHONY: all debug clear
