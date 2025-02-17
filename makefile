smallsh: main.o commands.o builtins.o processes.o
	gcc -std=gnu99 -o smallsh main.o commands.o builtins.o processes.o

smallshdebug:
	gcc -std=gnu99 -o smallsh *.c -DDEBUG=1

main.o: main.c commands.h processes.h
	gcc -std=gnu99 -c main.c

commands.o: commands.c commands.h builtins.h processes.h
	gcc -std=gnu99 -c commands.c

builtins.o: builtins.c builtins.h
	gcc -std=gnu99 -c builtins.c

processes.o: processes.c processes.h
	gcc -std=gnu99 -c processes.c
