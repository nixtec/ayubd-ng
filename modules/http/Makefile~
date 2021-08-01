# Makefile
#
include ../../makefile.inc

TARGETS		:= mod_http.so

.PHONY: all clean

all: $(TARGETS)

# PHP (PH7) embedded engine
#mod_http.so: mod_http.o http11_parser.o http11_requests.o ../../lfq.o ../../prog.o ../../lib/PH7/ph7.o
#	$(CC) $(CFLAGS) -shared -o $@ $^

# Unqlite embedded DB with JX9 interface
#mod_http.so: mod_http.o http11_parser.o http11_requests.o ../../lfq.o ../../prog.o ../../lib/unqlite/unqlite.o
#	$(CC) $(CFLAGS) -shared -o $@ $^

mod_http.so: mod_http.o ../../ioqueue.o ../../ioqueue_socket.o ../../assoc_array_impl.o ../../utils.o
	$(CC) $(CFLAGS) -shared -o $@ $^

# without any prog (vm, etc.)
#mod_http.so: mod_http.o ../../lib/libaco/aco.o ../../lib/libaco/acosw.o ../../ioqueue.o ../../ioqueue_socket.o ../../assoc_array_impl.o ../../utils.o
#	$(CC) $(CFLAGS) -shared -o $@ $^

mod_http.o: mod_http.c mod_http.h
	$(CC) $(CFLAGS) -fpic -c -o $@ mod_http.c

http11_parser.o: http11_parser.c http11_parser.h
	$(CC) $(CFLAGS) -fpic -c -o $@ http11_parser.c

http11_requests.o: http11_requests.c http11_requests.h
	$(CC) $(CFLAGS) -fpic -c -o $@ http11_requests.c

lru.o: lru.c lru.h
	$(CC) $(CFLAGS) -fpic -c -o $@ lru.c

mph.o: mph.c mph.h
	$(CC) $(CFLAGS) -fpic -c -o $@ mph.c

%.c: %.rl
	$(RAGEL) -o $@ $^

clean:
	rm -f *.o $(TARGETS)
