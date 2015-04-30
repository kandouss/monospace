CC = g++
CFLAGS  = -g -std=c++11 -ffast-math -O3 -funroll-loops -Wall `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`

default: clean monospace obj_rm


monospace:  oflow.o ocvio.o
	$(CC)  $(CFLAGS) oflow.o ocvio.o  $(LIBS) -o monospace

oflow.o:  oflow.cpp oflow.hpp ocvio.hpp 
	$(CC) $(CFLAGS) -c oflow.cpp

ocvio.o: ocvio.cpp ocvio.hpp
	$(CC) $(CFLAGS) -c ocvio.cpp

# Template for adding dependency "dep.c"
#
#dep.o:  dep.cpp dep.hpp
#	$(CC) $(CFLAGS) -c dep.cpp
#
#% : %.cpp
#        g++ $(CFLAGS) $(LIBS) -o $@ $<

clean: 
	$(RM) monospace *.o *~

obj_rm:
	$(RM) *.o
