all: server submit

server: grading_thread.o queue.o simple-server.o create_commands.o recieve_file.o process_compile_error.o process_runtime_error.o process_diff.o
		gcc grading_thread.o create_commands.o recieve_file.o process_compile_error.o process_runtime_error.o process_diff.o queue.o simple-server.o -o server -lm

create_commands.o: create_commands.c structures.h
		gcc -c create_commands.c -o create_commands.o

recieve_file.o: recieve_file.c structures.h
		gcc -c recieve_file.c -o recieve_file.o

process_compile_error.o: process_compile_error.c structures.h
		gcc -c process_compile_error.c -o process_compile_error.o

process_runtime_error.o: process_runtime_error.c structures.h
		gcc -c process_runtime_error.c -o process_runtime_error.o

process_diff.o: process_diff.c structures.h
		gcc -c process_diff.c -o process_diff.o

grading_thread.o: grading_thread.c utils.h structures.h create_commands.c recieve_file.c process_compile_error.c process_runtime_error.c process_diff.c
		gcc -c grading_thread.c -o grading_thread.o

queue.o: queue.c
		gcc -c queue.c -o queue.o

simple-server.o: simple-server.c utils.h
		gcc -c simple-server.c -o simple-server.o

submit: simple-client.c
		gcc simple-client.c -o submit

clean: 
		rm -f *.o server submit