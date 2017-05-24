C=g++
CFLAGS=-std=c++11 -O3

a3search: a3search.o porter2_stemmer.o
	$(C) $(CFLAGS) a3search.o porter2_stemmer.o -o a3search
	tar -xvf syn.tar

porter2_stemmer.o: porter2_stemmer.cpp porter2_stemmer.h
	$(C) $(CFLAGS) -c porter2_stemmer.cpp

a3search.o: a3search.cpp a3search.h
	$(C) $(CFLAGS) -c a3search.cpp
clean:
	rm *.o a3search
