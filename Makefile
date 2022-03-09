all : mdu

mdu :  queue.o list.o mdu.o data.o
	gcc -lm -pthread -o mdu queue.o list.o mdu.o data.o

mdu.o : mdu.c data.h
	gcc -g -std=gnu11 -Wall  -c mdu.c

queue.o : queue.c queue.h 
	gcc -g -std=gnu11 -Wall  -c queue.c 

list.o : list.c list.h 
	gcc -g -std=gnu11 -Wall -c list.c

data.o : data.c data.h 
	gcc -g -std=gnu11 -Wall -c data.c

clean : 
	rm -rf *.o mdu