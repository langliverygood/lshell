lshell:lshell.o test.o
	gcc -c lshell.c -g -Wall -lreadline -ltermcap
	gcc -c test.c -g -Wall
	gcc -o lshell lshell.o test.o -g -Wall -lreadline -ltermcap
