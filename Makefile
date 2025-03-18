CC = gcc
CFLAGS += -Wall -Wextra -Wpedantic -Wconversion -Wformat -Wformat-signedness -Wsign-conversion
CFLAGS += -Wshadow -Wstrict-prototypes -Wundef -Wdouble-promotion -Wcast-qual -Wpointer-arith
CFLAGS += -Wredundant-decls -Wmissing-declarations -Wswitch -Wswitch-enum -Wlogical-op -Wstrict-overflow
CFLAGS += -Wnull-dereference -Wstack-protector -Wformat-overflow -Wimplicit-function-declaration
CFLAGS += -fstack-protector-strong -fstack-check -fsanitize=undefined -fno-omit-frame-pointer
CFLAGS += -std=c99 -ggdb3 -D_FORTIFY_SOURCE=2
CFLAGS += $(shell pkg-config --cflags libmodbus)
LDFLAGS += -fstack-protector-strong -fsanitize=undefined
LDFLAGS += $(shell pkg-config --libs libmodbus)

PREFIX ?= ./target/

TARGETS = sparkshift amptrack

.PHONY: all clean install

all: $(TARGETS)

%: %.o
	$(CC) $< -o $@ $(LDFLAGS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

install:
	install -m755 -Dt $(PREFIX)/bin/ $(TARGETS)

clean:
	rm -f $(TARGETS) *.o
