# Author: 		Ran Bao
# Date:			1/March/2015
# Description:	Makefile config

# Define compilers and other flags
CC = cc
CFLAGS = -Os -Wall -Wstrict-prototypes -Wextra -g -std=c99 -D_XOPEN_SOURCE
LIBS =
DEL = rm

all: router.out

# Compile
router.o: router.c config.h list.h pidlock.h routing_table.h
	$(CC) -c $(CFLAGS) $(LIBS) $< -o $@

config.o: config.c config.h
	$(CC) -c $(CFLAGS) $(LIBS) $< -o $@

list.o: list.c list.h
	$(CC) -c $(CFLAGS) $(LIBS) $< -o $@

pidlock.o: pidlock.c pidlock.h
	$(CC) -c $(CFLAGS) $(LIBS) $< -o $@

router_demon.o: router_demon.c routing_table.h
	$(CC) -c $(CFLAGS) $(LIBS) $< -o $@

routing_table.o: routing_table.c routing_table.h
	$(CC) -c $(CFLAGS) $(LIBS) $< -o $@

# Link
router.out: router.o config.o list.o pidlock.o router_demon.o routing_table.o
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

.PHONY: clean
clean:
	-$(DEL) *.o *.out
