CXX=-Wall -pedantic -Werror
SRC_OBJS=$(wildcard *.c)
all: main.c
	gcc $(CXX) $(SRC_OBJS) -o dw
