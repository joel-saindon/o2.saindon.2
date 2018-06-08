proj2: main.c
	gcc -c main.c
	gcc main.o -o proj2

clean: 
	rm *.o proj2 
