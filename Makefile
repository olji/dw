CFLAGS=-Wall -pedantic -Werror
SRC_OBJS=$(wildcard *.c)
LIBS=-lm -lconfuse
DEBUG=1
CVARS= -DDEBUG=$(DEBUG)
debug: main.c
	gcc $(CFLAGS) -g $(SRC_OBJS) $(LIBS) $(CVARS) -o dw
release: main.c
	gcc $(CFLAGS) $(SRC_OBJS) $(LIBS) -o dw
test: debug
	sh tests/run_tests.sh $(SETUP)
