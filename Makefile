CC = g++
CFLAGS = -Wall -c -g

main: main.o parse.o execute.o
	$(CC) main.o parse.o execute.o -o main

main.o: main.cpp main.hpp
	$(CC) $(CFLAGS) main.cpp

parse.o: parse.cpp parse.hpp types.hpp
	$(CC) $(CFLAGS) parse.cpp

execute.o: execute.cpp execute.hpp
	$(CC) $(CFLAGS) execute.cpp

clean:
	rm *.o main
