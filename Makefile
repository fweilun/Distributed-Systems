CXX = g++
CXXFLAGS = -Wall -Wextra -g -std=c++17
BINS = server client
OBJS = machine.o
all: $(BINS) 

server: server.cpp machine.o
	$(CXX) $(CXXFLAGS) server.cpp machine.o -o server

client: client.cpp machine.o
	$(CXX) $(CXXFLAGS) client.cpp machine.o -o client
	
machine: machine.cpp machine.hpp
	$(CXX) $(CXXFLAGS) -c machine.hpp -o machine.o

LogGenerator: LogGenerator.cpp LogGenerator.o
	$(CXX) $(CXXFLAGS) -c LogGenerator.cpp LogGenerator.o -o client
	
clean:
	rm -f $(BINS) $(OBJS)
