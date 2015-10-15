OBJS = dtmove_cv3.o
CFLAGS = -Wall -g `pkg-config --cflags --libs opencv`
CXXFLAGS = -Wall -g `pkg-config --cflags --libs opencv`
CC = gcc
CXX = g++
r_dtmove_cv3.out : $(OBJS)
	$(CXX) $(OBJS) $(CFLAGS) -o $@
hello.o : dtmove_cv3.cpp
clean :
	rm *.out $(OBJS)
install :
	sudo cp *.out /usr/local/bin
