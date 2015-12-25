OBJS = Cdtmove_cv3.o
CFLAGS = -Wall -g -std=c++11 `pkg-config --cflags --libs opencv`
CXXFLAGS = -Wall -g -std=c++11 `pkg-config --cflags --libs opencv`
LDFLAGS = -pthread
CC = gcc
CXX = g++
r_dtmove_cv3.out : $(OBJS)
	$(CXX) $(OBJS) $(CXXFLAGS) $(LDFLAGS) -o $@
hello.o : Cdtmove_cv3.cpp
clean :
	rm *.out *.o
install :
	sudo cp *.out /usr/local/bin
