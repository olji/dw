CXX=-Wall -pedantic -Werror
SRC_OBJS=$(wildcard *.c)
LIBS=-lm -lconfuse
all: main.c
	gcc $(CXX) $(SRC_OBJS) $(LIBS) -o dw
debug: main.c
	gcc $(CXX) -g $(SRC_OBJS) $(LIBS) -o dw
