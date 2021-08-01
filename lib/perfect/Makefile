CC		:= gcc
CFLAGS		:= -Wall -O2

.PHONY: clean
%.o:%.c
	gcc $(CFLAGS) -c $<

O = lookupa.o recycle.o perfhex.o perfect.o

perfect: $(O)
	$(CC) $(CFLAGS) -o $@ $(O) -lm

# DEPENDENCIES

lookupa.o : lookupa.c standard.h lookupa.h

recycle.o : recycle.c standard.h recycle.h

perfhex.o : perfhex.c standard.h lookupa.h recycle.h perfect.h

perfect.o : perfect.c standard.h lookupa.h recycle.h perfect.h


clean:
	rm -f *.o perfect
