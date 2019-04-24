all: compile

compile: outputFolder main.o init.o threads.o handlers.o
	mpicc -o pyrkon output/main.o output/init.o output/threads.o output/handlers.o

handlers.o: handlers.c
	mpicc -c -Wall -o output/handlers.o handlers.c

threads.o: threads.c
	mpicc -c -Wall -o output/threads.o threads.c

init.o: init.c 
	mpicc -c -Wall -o output/init.o init.c

main.o: main.c main.h
	mpicc -c -Wall -o output/main.o main.c 

outputFolder:
	mkdir output

clear: 
	rm -r output pyrkon

run: 
	mpirun -np 4 pyrkon

reset: clear compile

