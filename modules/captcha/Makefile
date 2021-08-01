# Make
CC		:= gcc
CFLAGS		:= -Wall -O2
CFLAGS		+= $(shell pkg-config --cflags MagickWand)
LDFLAGS		:=
LDFLAGS		+= $(shell pkg-config --libs MagickWand)
#TARGETS		:= text-effects captcha
TARGETS		:= captcha captchagen

all: $(TARGETS)


%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

%: %.o
	$(CC) -o $@ $^ $(LDFLAGS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o $(TARGETS)
