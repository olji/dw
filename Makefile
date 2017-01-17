CFLAGS=-Wall -pedantic -Werror
SRC_OBJS=$(wildcard *.c)
LIBS=-lm -lconfig
DEBUG=1
CVARS= -DDEBUG=$(DEBUG)
.PHONY= clean
debug: main.c
	gcc $(CFLAGS) -g $(SRC_OBJS) $(LIBS) $(CVARS) -o dw
release: main.c
	gcc $(CFLAGS) $(SRC_OBJS) $(LIBS) -o dw
test: debug
	sh tests/run_tests.sh $(SETUP)
clean:
	rm -f dw
	find . -name '*.log' -type f -delete
	find . -name '*.lst' -type f -delete
	find . -name '*.tmp' -type f -delete
	find . -name '*.core.*' -type f -delete
	find . -name '*.mleak' -type f -delete
	find . -name '*.log' -type f -delete
