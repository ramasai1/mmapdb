CC = g++
CFLAGS = -Wall -c -g -std=c++17 -O3

main: main.o parse.o execute.o lexer.o
	$(CC) main.o parse.o execute.o lexer.o -o main

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp

lexer.o: lexer.cpp lexer.hpp
	$(CC) $(CFLAGS) lexer.cpp lexer.hpp

parse.o: parse.cpp parse.hpp types.hpp
	$(CC) $(CFLAGS) parse.cpp parse.hpp types.hpp

execute.o: execute.cpp execute.hpp
	$(CC) $(CFLAGS) execute.cpp execute.hpp

clean:
	rm *.o *.gch main
