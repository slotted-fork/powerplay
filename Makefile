CC = gcc
CFLAGS += -Wall -Wextra -Wpedantic -Wconversion -Wformat-signedness -Wshadow
CFLAGS += -fsanitize=undefined -std=c99 -ggdb3
CFLAGS += $(shell pkg-config --cflags libmodbus)
LDFLAGS += -fsanitize=undefined
LDFLAGS += $(shell pkg-config --libs libmodbus)

PREFIX ?= .

TARGETS = sparkshift amptrack

.PHONY: all clean

all: $(TARGETS)

%: %.o
	$(CC) $< -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install:
	install -m755 -Dt $(PREFIX)/bin/ $(TARGETS)

clean:
	rm -f $(TARGETS) *.o
