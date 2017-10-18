# the compiler: gcc for C program, define as g++ for C++
CC = g++
CFLAGS  = -g
all: knocker backdoor

knocker:
	$(CC) $(CFLAGS) -o knocker knocker.cpp

backdoor:
	$(CC) $(CFLAGS) -o backdoor backdoor.cpp

clean:
	$(RM) knocker backdoor *.o *~