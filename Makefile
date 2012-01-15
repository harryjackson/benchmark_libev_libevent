CC=gcc 

LIBS=-L./lib/lib
INCLUDES=-I./lib/include/ -I./lib/include 
LINKER_OPTS=-Wl,-rpath=./lib/lib

CFLAGS=-O3 -Wall -g -fno-guess-branch-probability 
#-ffast-math -funroll-all-loops -msse3 -march=native -mtune=native -fomit-frame-pointer 

SRCS=./*.c

objs=bench.o

all: depend bench

bench: $(objs)
	$(CC) $(CFLAGS) -o bench $(objs) $(INCLUDES) $(LIBS) -levent -lev $(LINKER_OPTS)

%.o:%.c
	$(CC) $(CFLAGS) -c $< $(INCLUDES) $(LINKER_OPTS)

depend: .depend
.depend: $(SRCS)
	-rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^ >> ./.depend;

include .depend

.PHONY : clean bench depend all 
clean:
	-rm -f bench callgrind* $(objs)
