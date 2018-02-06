#------------------------------------------------------------------------------

CFLAGS = -lpcap
CC=g++
MODE=sudo

#------------------------------------------------------------------------------

all: viewer

### 

viewer: viewer.cpp
	$(MODE) $(CC) -o viewer viewer.cpp $(CFLAGS)

clean:

	rm -f viewer

###

