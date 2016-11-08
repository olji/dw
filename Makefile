CXX=-Wall -pedantic -Werror
SRC_OBJS=$(wildcard *.c)
LIBS=-lm -lconfuse
debug: main.c
	gcc $(CXX) -g $(SRC_OBJS) $(LIBS) -o dw
release: main.c
	gcc $(CXX) $(SRC_OBJS) $(LIBS) -o dw
