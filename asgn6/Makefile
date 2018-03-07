CFLAGS = -Wall -pedantic -O3

all: mush

debug: CFLAGS += -g
debug: mush

mush: mush.o parseline.o checker.o
	gcc $(CFLAGS) -o mush $^

parseline: parseline.o checker.o
	gcc $(CFLAGS) -o parseline $^

mush.o: mush.c
	gcc $(CFLAGS) -c $<

parseline.o: parseline.c
	gcc $(CFLAGS) -c $<

checker.o: checker.c
	gcc $(CFLAGS) -c $<

clean:
	ls -al *.o
	rm -f *.o

test:
	make mush
	~pn-cs357/demos/tryAsgn6
