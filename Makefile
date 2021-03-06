CC=gcc
CPPFLAGS=-D_DEFAULT_SOURCE -I.
CFLAGS=-W -Wall -Wextra -Wshadow -std=c99

LD=gcc
LDFLAGS=
LIBS=-lXss -lX11 -lasound

HDR=$(wildcard src/*.h)
SRC=$(wildcard src/*.c)
OBJ=$(SRC:src/%.c=obj/%.o)
EXTRA_LIB=desktop-utils/libdesktop-utils.a
BIN=dwm-status

DEBUG ?= 0

ifneq ($(DEBUG), 0)
	CPPFLAGS += -DDEBUG=$(DEBUG)
	CFLAGS += -O0 -g
else
	CFLAGS += -O2
endif

.PHONY : all clean install uninstall
.SECONDARY : $(OBJ)


all : $(BIN)

$(BIN) : $(OBJ) $(EXTRA_LIB)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

obj/%.o : src/%.c $(HDR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

$(EXTRA_LIB) :
	make -C $(shell dirname $@)

clean :
	rm -f $(OBJ) $(BIN)
	make -C $(shell dirname $(EXTRA_LIB)) $@

install :
	mkdir -p $(HOME)/local/bin
	cp $(BIN) $(HOME)/local/bin

uninstall :
	rm -f $(HOME)/local/bin/$(BIN)
