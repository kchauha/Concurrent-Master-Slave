CC=gcc
CCOPTS=-g


TARGETS= server1 client1

all: $(TARGETS)

server1: server1.c
	$(CC) $(CCOPTS) -o $@ $^ $(LIB)

client1: client1.c
	$(CC) $(CCOPTS) -o $@ $^

clean:
	rm -f $(TARGETS) *.o
