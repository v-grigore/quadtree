OBJS	= main.o queue.o
SOURCE	= main.c queue.c
HEADER	= queue.h
OUT	= quadtree
CC	 = gcc
FLAGS	 = -g -c -Wall

build: $(OBJS)
	$(CC) $(OBJS) -o $(OUT) $(LFLAGS)

main.o: main.c
	$(CC) $(FLAGS) main.c 

queue.o: queue.c
	$(CC) $(FLAGS) queue.c 


clean:
	rm -f $(OBJS) $(OUT)