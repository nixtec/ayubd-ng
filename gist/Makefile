CC		:= gcc
CFLAGS		:= -Wall -O2
CFLAGS		+= $(shell pkg-config --cflags yaml-0.1)
LDFLAGS		:= -lm -ldl
LDFLAGS		+= $(shell pkg-config --libs yaml-0.1)
TARGETS		:= yaml-parse
#all: emit scan parse

all: $(TARGETS)

emit.o: emit.c
	gcc -c emit.c -g -O0 -Wall

emit: emit.o
	gcc -o emit emit.o -lyaml

scan.o: scan.c
	gcc -c scan.c -g -O0 -Wall

scan: scan.o
	gcc -o scan scan.o -lyaml

yaml-parse.o: yaml-parse.c
	$(CC) $(CFLAGS) -c -o $@ $^

yaml-parse: yaml-parse.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGETS) *.o
