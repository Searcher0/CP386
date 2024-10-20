# Do not edit the contents of this file.
CC = gcc
CFLAGS = -Werror -Wall -g  -std=gnu99
LDFLAGS = -lrt 
TARGET =  collatz_sequence process_management 
OBJFILES =  collatz_sequence.o process_management.o 

all:  $(TARGET)

process_management: process_management.c
	$(CC) $(CFLAGS)  -o process_management process_management.c $(LDFLAGS)

collatz_sequence: collatz_sequence.c
	$(CC) $(CFLAGS)  -o collatz_sequence  collatz_sequence.c

runq1: collatz_sequence
	./collatz_sequence start_numbers.txt
	
runq2: process_management
	./process_management sample_in_process.txt 

clean: *.c
	rm -rf collatz_sequence process_management  *.exe *.dSYM output.txt output_ref.txt *.out

