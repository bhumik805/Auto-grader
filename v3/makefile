all: server submit

server: grading_thread.o queue.o simple-server.o create_commands.o 
		gcc grading_thread.o create_commands.o queue.o simple-server.o -o server -lm

create_commands.o: create_commands.c structures.h
		gcc -c create_commands.c -o create_commands.o

grading_thread.o: grading_thread.c utils.h structures.h create_commands.c
		gcc -c grading_thread.c -o grading_thread.o

queue.o: queue.c
		gcc -c queue.c -o queue.o

simple-server.o: simple-server.c utils.h
		gcc -c simple-server.c -o simple-server.o

submit: simple-client.c
		gcc simple-client.c -o submit

clean: 
		rm -f *.o server submit

