CFLAGS = -Wall -g
all: part1 part2input part2fork
part1: part1test.o
	gcc $(CFLAGS) part1test.o -o part1
part1test.o: part1test.c
	gcc $(CFLAGS) -c part1test.c
part2input: part2input.o
	gcc $(CFLAGS) part2input.o -o part2input
part2input.o: part2input.c
	gcc $(CFLAGS) -c part2input.c
part2fork: part2fork.o
	gcc $(CFLAGS) part2fork.o -o part2fork
part2fork.o: part2fork.c
	gcc $(CFLAGS) -c part2fork.c
clean:
	rm -f *.o part1	
	rm -f *.o part2input
	rm -f *.o part2fork
