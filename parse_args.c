#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse_args.h"

static char *str_copy = NULL;

int parseArguments(char * const line, char **argv) {
	unsigned i = 0;

	// free string copy if we have used it before
	if (str_copy != NULL) {
		free(str_copy);
		str_copy = NULL;
	}

	// We create a copy of the input line, which we'll pass to strtok.
	// It's important to make a copy since strtok will modify the string
	// that it is tokenizing.
	str_copy = strndup(line, MAXLINE);

	char *token = strtok(str_copy, " \t\n");
	while (token != NULL && i < MAXARGS-1) {
		argv[i] = token;
		token = strtok(NULL, " \t\n");
		++i;
	}
	
	if (token != NULL) {
		fprintf(stderr, "ERROR: Exceeded maximum number of arguments\n");
		exit(1);
	}

	if (i == 0) {
		argv[0] = NULL;
		return 0;
	}
	else if (strcmp(argv[i-1], "&") == 0) {
		argv[i-1] = NULL;
		return 1;
	}
	else {
		argv[i] = NULL;
		return 0;
	}
}

int parsePath(char * const line, char **argv) {
	unsigned i = 0;

	if (str_copy != NULL) {
		free(str_copy);
		str_copy = NULL;
	}

	str_copy = strndup(line, MAXLINE);

	char *token = strtok(str_copy, ":");
	while (token != NULL && i < MAXARGS-1) {
		argv[i] = token;
		token = strtok(NULL, ":");
		i++;
	}
	if (token != NULL) {
		fprintf(stderr, "ERROR: Exceeded maximum number of arguments\n");
		exit(1);
	}

	if (i==0) {
		argv[0] = NULL;
		return 0;
	}
	else {
		argv[i] = NULL;
		return 0;
	}
}
