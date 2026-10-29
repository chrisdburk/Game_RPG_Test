# Simple build without CMake:  make  &&  ./rpg
CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic

SOURCES := $(shell find src -name '*.cpp')
HEADERS := $(shell find src -name '*.h')

rpg: $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -Isrc $(SOURCES) -o $@

clean:
	rm -f rpg

.PHONY: clean
