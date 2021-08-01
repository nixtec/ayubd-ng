# Makefile
# written by Ayub <mrayub@gmail.com>
# vim: sw=2
include makefile.inc

#TARGETS		:= ayubd assoc_array_test
TARGETS		:= ayubd
#TARGETS		:= captcha_static_svc
MODULES		:= http
#TARGETS		:= str_tokens

#CFLAGS		+= -DTEST_LRU_CACHE
#TARGETS		+= lru-cache

.PHONY: all clean modules

all: $(TARGETS) modules

#ayubd: lfq.o main.o assoc_array.o
#	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

assoc_array_test: assoc_array_test.c assoc_array_impl.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

modules:
	for dir in modules/http; do $(MAKE) -C $$dir clean all; done

producer-consumer-test: producer-consumer-test.c lfq.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# with coroutine
#ayubd: lfq.o main.o lib/libaco/aco.o lib/libaco/acosw.o
#	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

ayubd: main.o ioqueue.o ioqueue_socket.o assoc_array_impl.o utils.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

#ayubd: main.o lib/libaco/aco.o lib/libaco/acosw.o ioqueue.o ioqueue_socket.o assoc_array_impl.o utils.o
#	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

captcha_static_svc: lfq.o main.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

#ayubd: lfq.o main.o prog.o lib/PH7/ph7.o
#	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

#ayubd: lfq.o main.o prog.o lib/unqlite/unqlite.o
#	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

prog.o: prog.c prog.h
	$(CC) $(CFLAGS) -fpic -c -o $@ prog.c

lib/PH7/ph7.o: lib/PH7/ph7.c
	$(CC) $(CFLAGS) -fpic -c -o $@ $^

lib/unqlite/unqlite.o: lib/unqlite/unqlite.c
	$(CC) $(CFLAGS) -fpic -c -o $@ $^

lib/libaco/aco.o: lib/libaco/aco.c
	$(CC) $(CFLAGS) -fpic -c -o $@ $^

lib/libaco/acosw.o: lib/libaco/acosw.S
	$(CC) $(CFLAGS) -fpic -c -o $@ $^

ioqueue.o: ioqueue.c
	$(CC) $(CFLAGS) -fpic -c -o $@ $^

ioqueue_socket.o: ioqueue_socket.c
	$(CC) $(CFLAGS) -fpic -c -o $@ $^

lfq-test: lfq.o lfq-test.c 
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

main.o: main.h main.c poll.h lib/libaco/aco.c
	$(CC) $(CFLAGS) -c -o $@ main.c

lfq.o: lfq.h lfq.c
	$(CC) $(CFLAGS) -fpic -c -o $@ lfq.c


utils.o: utils.c
	$(CC) $(CFLAGS) -fpic -c -o $@ $^

assoc_array.o: assoc_array.c
	$(CC) $(CFLAGS) -fpic -c -o $@ $^

assoc_array_impl.o: assoc_array_impl.c
	$(CC) $(CFLAGS) -fpic -c -o $@ $^


config: config.c config.h
	$(CC) $(CFLAGS) -o $@ config.c ext-lib/ini_parser/minIni.c

clean:
	rm -f *.o $(TARGETS)
