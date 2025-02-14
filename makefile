smallsh: main.o commands.o
	gcc -std=gnu99 -o smallsh main.o commands.o

main.o: main.c commands.h
	gcc -std=gnu99 -c main.c

commands.o: commands.c commands.h
	gcc -std=gnu99 -c commands.c
