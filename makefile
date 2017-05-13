C=g++
CFLAGS=-std=c++11 -O3
all: a3search

a3search: a3search.h a3search.cpp porter2_stemmer.h porter2_stemmer.cpp
	$(C) $(CFLAGS) -o a3search a3search.cpp porter2_stemmer.cpp

clean:
	rm a3search