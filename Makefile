PROGRAMS = main

# Important optimization options
CFLAGS = -O3 -ffast-math -fno-rtti
CXXFLAGS = -std=c++11
# Standard libraries
LFLAGS = -lm -lstdc++ -lpthread -pthread

# Debugging
CFLAGS += -g -Wall
LFLAGS += -g

# Annoying warnings on by default on Mac OS
CFLAGS += -Wno-tautological-constant-out-of-range-compare -Wno-gnu-static-float-init


all: $(PROGRAMS)

.cpp:
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS)

.PHONY: clean all

clean:
	rm -f $(PROGRAMS)
