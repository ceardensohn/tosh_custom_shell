/*
 * COMP 280 @ USD
 *
 * Header file for parse_args library.
 *
 * DO NOT MODIFY THIS FILE IN ANY WAY!!!
 */

#ifndef __PARSE_ARGS_H__
#define __PARSE_ARGS_H__

#define MAXLINE    1024   // max command line size 
#define MAXARGS     128   // max number of arguments on a command line 

/* 
 * Parse the command line and build the argv array.
 *
 * @param line The command line string entered at the shell prompt
 * @param argv An array of size MAXARGS of char *
 *           parseArguments will initialize its contents from the passed
 *           cmdline string.
 *           The caller should pass in a variable delcared as:
 *              char *argv[MAXARGS];
 *              (ex) int bg = parseArguments(commandLine, argv);
 *
 *           argv will be filled with one string per command line
 *           argument.  The first argv[i] value following the last
 *           command line string will be NULL.  For example:
 *              ls -l     argv[0] will be "ls"  argv[1] will be "-l"
 *                        argv[2] will be NULL
 *          for an empty command line, argv[0] will be set to NULL
 *
 *  @return non-zero (true) if the command line includes & at the end
 *                             to run the command in the background
 *             zero (false) for a foreground command foreground
 *
 *  @note The contents of argv will be invalidated the next time you call this
 *  function so be careful not to rely on it through multiple calls.
 */
int parseArguments(char * const line, char **argv);
int parsePath(char * const line, char **argv);
#endif
