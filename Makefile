CC = gcc
CFLAGS = -Wall -g
BINS = game libnetworking.a runtime_game run
all: $(BINS)

libnetworking.o: libnetworking.c networking.h
	$(CC) $(CFLAGS) -c libnetworking.c

libnetworking.a: libnetworking.o
	ar rcs libnetworking.a libnetworking.o

game: game.c libnetworking.o
	$(CC) $(CFLAGS) -o $@ $^

runtime_game: game.c libnetworking.o
	$(CC) $(CFLAGS) -o $@ $^ -L. -lnetworking

clean:
	rm *.o $(BINS)

run:
	./game