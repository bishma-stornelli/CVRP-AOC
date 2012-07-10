CC=gcc
OPTIONS=-Wall -Wextra
OBJECTS=aoc.o cvrp_instance.o

test: main.o cvrp_instance.o aoc.o TSP-TEST.V0.9/instance.o TSP-TEST.V0.9/utilities.o TSP-TEST.V0.9/timer.o TSP-TEST.V0.9/ls.o
	$(CC) -O main.o cvrp_instance.o aoc.o TSP-TEST.V0.9/timer.o TSP-TEST.V0.9/instance.o TSP-TEST.V0.9/utilities.o TSP-TEST.V0.9/ls.o -o cvrp -lm -ansi -Wall

main.o: main.c $(OBJECTS)
	$(CC) $(OPTIONS) -O -c main.c -o main.o 

TSP-TEST.V0.9/timer.o: TSP-TEST.V0.9/timer.h TSP-TEST.V0.9/timer.c
	$(CC) -O -c TSP-TEST.V0.9/timer.c -o TSP-TEST.V0.9/timer.o -ansi -Wall

TSP-TEST.V0.9/instance.o: TSP-TEST.V0.9/instance.h TSP-TEST.V0.9/instance.c
	$(CC) -O -c TSP-TEST.V0.9/instance.c -o TSP-TEST.V0.9/instance.o  -Wall

TSP-TEST.V0.9/utilities.o: TSP-TEST.V0.9/utilities.h TSP-TEST.V0.9/utilities.c
	$(CC) -O -c TSP-TEST.V0.9/utilities.c -o TSP-TEST.V0.9/utilities.o -ansi -Wall

TSP-TEST.V0.9/ls.o: TSP-TEST.V0.9/ls.h TSP-TEST.V0.9/ls.c
	$(CC) -O -c TSP-TEST.V0.9/ls.c -o TSP-TEST.V0.9/ls.o -ansi -Wall

%.o: %.c %.h
	$(CC) $(OPTIONS) -c $<

clean:
	rm *.o cvrp