CC=gcc
CCOPTS=-g
LIB = -lpthread

TARGETS=MultiServiceServer TCPclient UDPclient

all: $(TARGETS)

MultiServiceServer: MultiServiceServer.c
	$(CC) $(CCOPTS) -o $@ $^ $(LIB)

TCPclient: TCPclient.c
	$(CC) $(CCOPTS) -o $@ $^

UDPclient: UDPclient.c
	$(CC) $(CCOPTS) -o $@ $^

clean:
	rm -f $(TARGETS) *.o
