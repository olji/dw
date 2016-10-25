CXX=-Wall -pedantic -Werror
SRC_OBJS=$(wildcard *.c)
all: main.c
	gcc $(CXX) $(SRC_OBJS) -o dw
debug: main.c
	gcc $(CXX) -g $(SRC_OBJS) -o dw
check: debug
	valgrind --track-origins=yes --leak-check=full -v ./dw -c report.tex TABLE2
leak: debug
	valgrind --leak-check=full ./dw -c report.tex TABLE2
