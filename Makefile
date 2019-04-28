#gmodule = -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/ -I/usr/include/glib-2.0/
mpiCompiler = mpic++

all: compile

compile: outputFolder main.o init.o threads.o handlers.o
	$(mpiCompiler) -o pyrkon output/main.o output/init.o output/threads.o output/handlers.o

handlers.o: handlers.cpp
	$(mpiCompiler) -c -Wall -o output/handlers.o handlers.cpp

threads.o: threads.cpp
	$(mpiCompiler) -c -Wall -o output/threads.o threads.cpp

init.o: init.cpp 
	$(mpiCompiler) -c -Wall -o output/init.o init.cpp

main.o: main.cpp main.h
	$(mpiCompiler) -c -Wall -o output/main.o main.cpp 

outputFolder:
	mkdir output

clear: 
	rm -f -r output pyrkon

run: 
	mpirun -np 4 pyrkon

reset: clear compile

