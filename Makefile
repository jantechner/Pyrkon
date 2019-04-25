#gmodule = -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/ -I/usr/include/glib-2.0/
gmodule = 

all: compile

compile: outputFolder main.o init.o threads.o handlers.o
	mpicc -o pyrkon output/main.o output/init.o output/threads.o output/handlers.o

handlers.o: handlers.c
	mpicc -c -Wall $(gmodule) -o output/handlers.o handlers.c

threads.o: threads.c
	mpicc -c -Wall $(gmodule) -o output/threads.o threads.c

init.o: init.c 
	mpicc -c -Wall $(gmodule) -o output/init.o init.c

main.o: main.c main.h
	mpicc -c -Wall $(gmodule) -o output/main.o main.c 

outputFolder:
	mkdir output

clear: 
	rm -f -r output pyrkon

run: 
	mpirun -np 4 pyrkon

reset: clear compile

