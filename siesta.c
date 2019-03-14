//  A long running program for testing the Tiny Torero shell program.
//  It can run with or without a command line argument that specifies
//  how many times the program should sleep for 2 seconds  before 
//  exiting.  With no command line argument it does this 5 times.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
	long i, num = 5;
	if(argc == 2) {
		num = strtol(argv[1], NULL, 10);
	}
	else if (argc > 2) {
		fprintf(stderr, "usage %s [number of 2 second intervals to sleep]\n", argv[0]);
		exit(1);
	}


	for(i=0; i < num; i++) {
		sleep(2);
	}
	
	
	return 0;
}
