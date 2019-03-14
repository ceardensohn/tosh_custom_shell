/*
 * Header file for circular queue of HistoryListItems.
 * 
 * Used from COMP280 starter code
 */
#ifndef __HISTQUEUE_H__
#define __HISTQUEUE_H__

#include "parse_args.h"

#define MAXHIST 10   // max number of commands in history list

/*
 * A struct to keep information one command in the history of 
 * command executed
 */
struct HistoryEntry {              
    unsigned int cmd_num;
    char cmdline[MAXLINE]; // command line for this process
};

// You can use "HistoryEntry" instead of "struct HistoryEntry"
typedef struct HistoryEntry HistoryEntry;


void add_to_history(char * const cmd);
void print_history();
char* get_command(unsigned int cmd_num);
char* get_last_command();
#endif
