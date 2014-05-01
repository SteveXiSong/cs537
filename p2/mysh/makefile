#objs = mysh.o

#test:$(objs)	
#	gcc -o test $(objs)
mysh.o:mysh.c cmd.c mysh.h 
	gcc mysh.c cmd.c -o mysh  -g -Wall -Werror


.PHONY:clean
clean: 
	rm -f *.o
	rm -f mysh

