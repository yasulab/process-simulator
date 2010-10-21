main.out: main.o
	gcc -o ProcessSimulator main.o
main.o: main.c
	gcc -c main.c
clean:
	rm -f ProcessSimulator main.o
run:
	make
	./ProcessSimulator init.prog
