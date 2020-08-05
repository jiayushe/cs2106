CC=gcc
CFLAGS=-Wall -fPIC
LDFLAGS = -L.
LDLIBS = -lmy_stdio -ldl

all: libmy_stdio.so

libmy_stdio.so: ex1.o ex2.o ex3.o ex4.o
	$(CC) -shared -o $@ $^

my_stdio.o: ex1.c ex2.c ex3.c ex4.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f libmy_stdio.so runner ex*.o demo
