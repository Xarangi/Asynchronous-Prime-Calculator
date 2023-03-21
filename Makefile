CC = g++
CD = gcc

all: primes primes1 primes2

primes: primes.o functions.o
	$(CC) -o primes primes.o functions.o
primes.o: primes.cpp headers.h
	$(CC) -c primes.cpp
functions.o: functions.cpp headers.h
	$(CC) -c functions.cpp
primes1: primes1.o
	$(CD) -o primes1 primes1.o
primes1.o: primes1.c
	$(CD) -c primes1.c
primes2: primes2.o
	$(CD) -o primes2 primes2.o -lm
primes2.o: primes2.c
	$(CD) -c primes2.c -lm

clean:
	rm -f primes2.o primes1.o primes.o functions.o primes primes1 primes2


