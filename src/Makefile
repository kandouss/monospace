CC = gcc
CGLAGS = -g -O3 -ffast-math -Wall -std=c99

default: clean main obj_rm

main: main.o msp_input_v4l2.o msp_output_sdl.o msp_buffer.o
	$(CC) main.o msp_input_v4l2.o msp_output_sdl.o msp_buffer.o -o main -lSDL

main.o: main.c msp_config.h msp_input_v4l2.h msp_output_sdl.h msp_buffer.h
	$(CC) $(CFLAGS) -c main.c

msp_input_v4l2.o: msp_input_v4l2.c msp_config.h msp_input_v4l2.h msp_buffer.h
	$(CC) $(CFLAGS) -c msp_input_v4l2.c

msp_output_sdl.o: msp_output_sdl.c msp_config.h msp_output_sdl.h msp_buffer.h
	$(CC) -I/usr/include/SDL $(CFLAGS) -c msp_output_sdl.c

msp_buffer.o: msp_buffer.c msp_config.h msp_buffer.h
	$(CC) $(CFLAGS) -c msp_buffer.c

# Template for adding dependency "dep.c"
#
#dep.o:  dep.cpp dep.hpp
#	$(CC) $(CFLAGS) -c dep.cpp
#
#% : %.cpp
#        g++ $(CFLAGS) $(LIBS) -o $@ $<

clean: 
	$(RM) cam *.o 

obj_rm: 
	$(RM) *.o
