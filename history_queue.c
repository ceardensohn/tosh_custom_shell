/*
 * The Tiny Torero Shell (TTSH)
 *
 * Used from COMP280 Starter code
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "history_queue.h"

static HistoryEntry history[MAXHIST]; 

static int next_index;
static int start_index;
static int list_size;
static int history_num;

void add_to_history(char * const cmd) {
	// add the entry at the next spot
	history[next_index].cmd_num = history_num;
	history_num++;
	strncpy(history[next_index].cmdline, cmd, MAXLINE);

	// update where we'll put the next entry
	next_index = (next_index+1) % MAXHIST;

	// if we're at max capacity, we'll have to move the start index over to
	// account for overfilling.
	if (list_size >= MAXHIST) {
		start_index = (start_index+1) %  MAXHIST;
	}
	else {
		list_size++ ;
	}
}

void print_history() {
	for (int i = start_index, j = 0; j < list_size; j++, i = (i+1)%MAXHIST) {
		printf("%d\t%s\n", history[i].cmd_num, history[i].cmdline);
	}
}

char* get_command(unsigned int cmd_num) {
	for (int i = start_index, j = 0; j < list_size; j++, i = (i+1)%MAXHIST) {
		if (history[i].cmd_num == cmd_num)             
				return history[i].cmdline;
	}
	return NULL;
}

char* get_last_command() {
	char *cmd = get_command(((unsigned int)list_size - 1));
	return cmd;
}
