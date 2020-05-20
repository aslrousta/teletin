# Makefile
#
# Copyright (c) 2020 Ali AslRousta <aslrousta@gmail.com>
# See LICENSE file.
#

CFLAGS?=-O2 -Wall -Werror
LDFLAGS?=

LDFLAGS+= -lev

PROJECT=teletin
OBJECTS=$(patsubst %.c,%.o,$(wildcard src/*.c))

all: $(PROJECT)

$(PROJECT): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) -f $(PROJECT) $(OBJECTS)

.PHONEY: all
