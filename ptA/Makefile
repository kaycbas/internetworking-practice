#------------------------------------------------------------------------------

CFLAGS= -lws2_32 -lpthread -std=c++11
CC=g++

#------------------------------------------------------------------------------

all: server client

### 

server: server.cpp
	$(CC) -o server server.cpp $(CFLAGS)

client: client.cpp
	$(CC) -o client client.cpp $(CFLAGS)

clean:

	rm -f server

###


