OBJS = dtmove_cv3.o
CFLAGS = -Wall -g `pkg-config --cflags --libs opencv`
CXXFLAGS = -Wall -g `pkg-config --cflags --libs opencv`
CC = gcc
CXX = g++
dtmove_cv3 : $(OBJS)
	$(CXX) $(OBJS) $(CFLAGS) -o $@
hello.o : dtmove_cv3.cpp
clean :
	rm dtmove_cv3 $(OBJS)
install :
	sudo cp dtmove_cv3 /usr/local/bin
