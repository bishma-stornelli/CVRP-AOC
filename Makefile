
test: main.o TSP-TEST.V0.9/instance.o TSP-TEST.V0.9/utilities.o TSP-TEST.V0.9/timer.o TSP-TEST.V0.9/ls.o
	gcc -O main.o TSP-TEST.V0.9/timer.o TSP-TEST.V0.9/instance.o TSP-TEST.V0.9/utilities.o TSP-TEST.V0.9/ls.o -o stsp -lm -ansi -Wall

main.o: main.c
	gcc -O -c main.c -o main.o -ansi -Wall

TSP-TEST.V0.9/timer.o: TSP-TEST.V0.9/timer.h TSP-TEST.V0.9/timer.c
	gcc -O -c TSP-TEST.V0.9/timer.c -o TSP-TEST.V0.9/timer.o -ansi -Wall

TSP-TEST.V0.9/instance.o: TSP-TEST.V0.9/instance.h TSP-TEST.V0.9/instance.c
	gcc -O -c TSP-TEST.V0.9/instance.c -o TSP-TEST.V0.9/instance.o -ansi -Wall

TSP-TEST.V0.9/utilities.o: TSP-TEST.V0.9/utilities.h TSP-TEST.V0.9/utilities.c
	gcc -O -c TSP-TEST.V0.9/utilities.c -o TSP-TEST.V0.9/utilities.o -ansi -Wall

TSP-TEST.V0.9/ls.o: TSP-TEST.V0.9/ls.h TSP-TEST.V0.9/ls.c
	gcc -O -c TSP-TEST.V0.9/ls.c -o TSP-TEST.V0.9/ls.o -ansi -Wall

