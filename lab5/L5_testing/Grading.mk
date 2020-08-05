CC=gcc
CFLAGS=-Wall
LDFLAGS = -L.
LDLIBS = -lmy_stdio -ldl

all: read_buffered read_operations read_misc write_buffered write_operations write_misc fseek fflush

read_buffered: read_buffered.c util.c hooks.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

read_operations: read_operations.c util.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

read_misc: read_misc.c util.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

write_buffered: write_buffered.c util.c hooks.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

write_operations: write_operations.c util.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

write_misc: write_misc.c util.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

fseek: fseek.c util.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

fflush: fflush.c util.c hooks.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f read_buffered read_operations read_misc write_buffered write_operations write_misc fseek fflush