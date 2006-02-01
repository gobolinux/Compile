all:
	gcc -o SuperUserName SuperUserName.c
	gcc -o IsExecutable IsExecutable.c
	gcc -o usleep usleep.c
	gcc -o LinkOrExpandAll LinkOrExpandAll.c -std=c99
	mv SuperUserName IsExecutable usleep ../bin/ 
static:	
	gcc -o SuperUserName SuperUserName.c -static
	gcc -o IsExecutable IsExecutable.c -static
	gcc -o usleep usleep.c -static
	gcc -o LinkOrExpandAll LinkOrExpandAll.c -std=c99 -static
	mv SuperUserName IsExecutable usleep ../bin/ 
