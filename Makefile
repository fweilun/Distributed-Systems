.PHONY: all test clean

CXX      = g++
CXXFLAGS = -Wall -Wextra -g -std=c++17 -pthread

TESTDIR = test
BINDIR  = bins
LOGDIR  = logs

BINS = $(BINDIR)/server $(BINDIR)/client \
       $(BINDIR)/LogGenerator $(BINDIR)/UnitTest
OBJS = $(BINDIR)/machine.o

all: $(BINS)

$(BINDIR) $(LOGDIR):
	mkdir -p $@

$(BINDIR)/machine.o: machine.cpp machine.hpp | $(BINDIR)
	$(CXX) $(CXXFLAGS) -c machine.cpp -o $@

$(BINDIR)/server: server.cpp machine.hpp $(BINDIR)/machine.o | $(BINDIR)
	$(CXX) $(CXXFLAGS) server.cpp $(BINDIR)/machine.o -o $@

$(BINDIR)/client: client.cpp machine.hpp $(BINDIR)/machine.o | $(BINDIR)
	$(CXX) $(CXXFLAGS) client.cpp $(BINDIR)/machine.o -o $@

$(BINDIR)/LogGenerator: $(TESTDIR)/LogGenerator.cpp | $(BINDIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BINDIR)/UnitTest: $(TESTDIR)/UnitTest.cpp | $(BINDIR)
	$(CXX) $(CXXFLAGS) $< -o $@

test: $(BINS) | $(LOGDIR)
	./$(BINDIR)/UnitTest

clean:
	rm -rf $(BINDIR) $(LOGDIR) $(OBJS)