#export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:~/private/OS537/p5/p5x4/
#before run

CC   = gcc
OPTS = -Wall -Werror
# src  =  libmfs.c main.c udp.c


all: client libmfs mymfs server

#libsrc = libmfs.c


# this generates the target executables
#server: server.o udp.o 
#	$(CC) -o server server.o udp.o

#servert: servert.c udp.o
#	$(CC) -o servert servert.o udp.o

server: server.o udp.o
	$(CC) -o server server.o udp.o -g

libmfs:	libmfs.c  udp.c
	$(CC) -c -fpic libmfs.c udp.c $(OPTS)
	$(CC) -shared -o libmfs.so libmfs.o udp.o

mymfs:  libmfs.so
	gcc -lmfs -L. -o mymfs main.c -Wall -Werror

# this is a generic rule for .o files 
%.o: %.c 
	$(CC) $(OPTS) -c $< -o $@

clean:
	rm -f *.o *.so 



