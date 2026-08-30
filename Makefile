.PHONY: test all clean
CXX = g++
CXXFLAGS = -Wall -Wextra -g -std=c++17  -pthread
BINS = server client run_test/LogGenerator run_test/UnitTest
OBJS = machine.o
all: $(BINS) 


server: server.cpp machine.o
	$(CXX) $(CXXFLAGS) server.cpp machine.o -o server

client: client.cpp machine.o
	$(CXX) $(CXXFLAGS) client.cpp machine.o -o client
	
machine: machine.cpp machine.hpp
	$(CXX) $(CXXFLAGS) -c machine.cpp -o machine.o

LogGenerator: run_test/LogGenerator.cpp 
	$(CXX) $(CXXFLAGS) run_test/LogGenerator.cpp -o run_test/LogGenerator

UnitTest: run_test/UnitTest.cpp 
	$(CXX) $(CXXFLAGS) run_test/UnitTest.cpp -o run_test/UnitTest

test: $(BINS)
	./run_test/UnitTest
	
clean:
	rm -f $(BINS) $(OBJS)
