all:
	gcc -o main -Wall -O3 -mavx2 main.c levenstein.c
