CXX=-Wall -pedantic -Werror
all: main.c
	gcc $(CXX) main.c -o dw
