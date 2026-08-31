.PHONY: all test clean

CXX      = g++
CXXFLAGS = -Wall -Wextra -g -std=c++17 -pthread

TESTDIR = test
BINDIR  = bins
LOGDIR  = logs

BINS = $(BINDIR)/server $(BINDIR)/client \
       $(BINDIR)/LogGenerator $(BINDIR)/UnitTest
OBJS = machine.o

all: $(BINS)

$(BINDIR) $(LOGDIR):
	mkdir -p $@

machine.o: machine.cpp machine.hpp
	$(CXX) $(CXXFLAGS) -c machine.cpp -o $@

$(BINDIR)/server: server.cpp machine.hpp machine.o | $(BINDIR)
	$(CXX) $(CXXFLAGS) server.cpp machine.o -o $@

$(BINDIR)/client: client.cpp machine.hpp machine.o | $(BINDIR)
	$(CXX) $(CXXFLAGS) client.cpp machine.o -o $@

$(BINDIR)/LogGenerator: $(TESTDIR)/LogGenerator.cpp | $(BINDIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BINDIR)/UnitTest: $(TESTDIR)/UnitTest.cpp | $(BINDIR)
	$(CXX) $(CXXFLAGS) $< -o $@

test: $(BINS) | $(LOGDIR)
	./$(BINDIR)/UnitTest

clean:
	rm -rf $(BINDIR) $(LOGDIR) $(OBJS)