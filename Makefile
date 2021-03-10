all: main.c levenstein.c
	gcc -o main -Wall -O3 -mavx2 main.c levenstein.c
