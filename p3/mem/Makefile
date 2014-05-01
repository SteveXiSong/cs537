#add more lib c files below
lib_src = mem_init.c mem_alloc.c 

all: lib_Mem tst_main
lib_Mem:	
	gcc -c -fpic $(lib_src) -Wall -Werror
	#dont change above line
	gcc -shared -o libmem.so mem_init.o mem_alloc.o
	#add *.o above

tst_main: 
	gcc -lmem -L. -o tst_main tst_main.c -Wall -Werror
#export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:~/private/OS537/p3/	
#before run
clean:
	rm -f *.o *.so tst_main

