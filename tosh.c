/*
 * The Torero Shell (TOSH)
 *
 * @author Chris Eardensohn ceardensohn@sandiego.edu
 * @author Anna Colman acolman@sandiego.edu
 * 
 * Description: Torero shell is an extension of ttsh that can run
 * simple cmd line instructions, running commands in the background,
 * and support for piping and IO redirection.
 */

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <sys/wait.h>
#include <readline/readline.h>
#include <signal.h>
#include <libgen.h>

#include "parse_args.h"
#include "history_queue.h"

static void handleCommand(char **args, int bg, char *cwd);
void runExternalCommand(char **args, int bg);
void parseAndExecute(char *cmdline, char **args, char *cwd);
void runCD(char **args, char *cwd);
void ioRedirect(char **args);
int check_if_pipe(char **args);
void call_pipe(char **args, int bg);
void run_pipe(int *pipe_fd, char **cmd1, char **cmd2, int bg);

void child_reaper(__attribute__ ((unused)) int sig_num) {
	while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main(){ 

	char *argv[MAXARGS];
	// register a signal handler for SIGCHLD
	struct sigaction sa;
	sa.sa_handler = child_reaper;
	sa.sa_flags = 0;
	sigaction(SIGCHLD, &sa, NULL);

	// get absolute path directory on start up
	char cwd[MAXLINE];
	getcwd(cwd, sizeof(cwd));

	while(1) {
		// (1) read in the next command entered by the user
		char *cmdline = readline("tosh$ ");

		// NULL indicates EOF was reached, which in this case means someone
		// probably typed in CTRL-d
		if (cmdline == NULL) {
			fflush(stdout);
			exit(0);
		}

		// (2) parse the cmdline, then determine how to execute it 
		parseAndExecute(cmdline, argv, cwd);

		// (3) determine how to execute it, and then execute it

	}
	return 0;
}

void parseAndExecute(char *cmdline, char **args, char *cwd) {
	
	// make a call to parseArguments function to parse it into its argv format
	int bg = parseArguments(cmdline, args);

	// determine how to execute it, and then execute it
	if (args[0] != NULL) {
		if (args[0][0] != '!')
			add_to_history(cmdline);
		handleCommand(args, bg, cwd);
	}
}

void handleCommand(char **args, int bg, char *cwd) {         
	
	// check for piping 
	if(check_if_pipe(args)) {
		call_pipe(args,  bg);
 	}

	// handle built-in directly
	// exit command
	else if (strcmp(args[0], "exit") == 0) {
		printf("Goodbye!\n");
		exit(0);
	}
	
	// cd
	else if (strcmp(args[0], "cd") == 0) {
		runCD(args, cwd);
	}

	// history commands
	else if (strcmp(args[0], "history") == 0) {
		print_history();
	}
	else if (strcmp(args[0], "!!") == 0) {
		char *cmd = get_last_command();
		if (cmd == NULL) {
			fprintf(stderr, "ERROR: no commands in history\n");
		}
		else {
			parseAndExecute(cmd, args, cwd);
		}
	}	
	else if (args[0][0] == '!') {
		unsigned int cmd_num = strtoul(&args[0][1], NULL, 10);
		char *cmd = get_command(cmd_num);
		if (cmd == NULL)
			fprintf(stderr, "ERROR: %d is not in history\n", cmd_num);
		else {
			parseAndExecute(cmd, args, cwd);
		}
	}
	// handle external command with our handy function
	else {
			runExternalCommand(args, bg);
		}
}

// find the executable path then call execv to execute the user command
void runExternalCommand(char **args, int bg) {
	
	// fork
	pid_t cpid = fork();

	// build the **char array of executable paths
	if (cpid  ==  0) {

		// check for IO redirect
		ioRedirect(args);
		if (access(args[0], X_OK) == 0) {
			execv(args[0], args);
		}
		
		// build char ** of paths and check for the correct path
		// if found, call exec, else else print error message
		char absolute_path[MAXLINE];	
		char *path = getenv("PATH");
		char *token = NULL;
		token = strtok(path, ":");
		while(token != NULL) {
			strcpy(absolute_path, token);	
			strcat(absolute_path, "/");
			strcat(absolute_path, args[0]);		
			if(access(absolute_path, X_OK) == 0) {	
				execv(absolute_path, args);			
			}
			token = strtok(NULL, ":");
		}
		// if we got to this point, execv failed!
		fprintf(stderr , "ERROR: Command not found\n");
		exit(63);
	}
	else if (cpid > 0) {
		// parent
		if (bg) {
			// Quick check if child has returned quickly.
			// Don't block here if child is still running though.
			waitpid(cpid, NULL, WNOHANG);
		}
		else {
			// wait here until the child really finishes
			waitpid(cpid, NULL, 0);
		}
	}
	else {
		// something went wrong with fork
		perror("fork");
		exit(1);
	}
}

// method to handle the cd command
void runCD(char **args, char *cwd) {

	// cd   command 
	if (args[1] == NULL) {

		strcpy(cwd, getenv("HOME"));
		chdir(cwd);

	}
	// cd .. command
	else if (strcmp(args[1], "..") == 0) {
		cwd = dirname(cwd);
		chdir(cwd);
	}
	// cd directory command
	else {

		char new_cwd[MAXLINE];
		strcpy(new_cwd, cwd);
		strcat(new_cwd, "/");
		strcat(new_cwd, args[1]);
		int res = chdir(new_cwd);
		if (res == -1) {
			printf("Error: Directory does not exist\n");
		}
		else {
			strcpy(cwd, new_cwd);
		}

	}
}
/**
 * function to redirect stdout, stderr, or stdin
 * @param args the command list arguments
 */
void ioRedirect(char **args){
	int i, out = 0, err = 0, in = 0;
	int fd0, fd1, fd2;
	char input[64], output[64], error[64];
	while(args[i] != NULL) {
		if(strcmp(args[i], "<") == 0) {
			strcpy(input, args[i+1]);
			in = 1;
		}
		if(strcmp(args[i], "1>") == 0) {
			strcpy(output, args[i+1]);
			out = 1;
		}
		if(strcmp(args[i], "2>") == 0) {
			strcpy(error, args[i+1]);
			err = 1;
		}
		if (in || out || err) {
			args[i] = NULL;
		}
		i++;		
	}	

	if (in) {
		fd0 = open(input, O_RDONLY);
		if (fd0 < 0) {
			perror("Error opening file\n");
		}
		dup2(fd0, STDIN_FILENO);
		close(fd0);
		in = 0;
	}
	if (out) {
		if ((fd1 = open(output, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
			perror("Open failed\n");
		}
		dup2(fd1, 1);
		close(fd1);
	}
	if (err) {
		if ((fd2 = open(error, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
			perror("Error opening output file\n");
		}
		dup2(fd2, STDERR_FILENO);
		close(fd2);
	}
}

// Check if we need to pipe
// if piping character is found, return its index
// else return 0
int check_if_pipe(char **args) {

	int i = 1;
	while(args[i] != NULL) {
		if(strcmp(args[i], "|") == 0) {
			return i;
		}
		i++;
	}
	return 0;
} 

/**
 * method to separate commands for piping calls pipe()
 * @param args the arguments from command line
 * @param bg background flag
 */ 
void call_pipe(char **args, int bg) {
	
	// 2d char arrays for commands 
	char *cmd1[MAXARGS];
	char *cmd2[MAXARGS];
	
	int i = 0;
	// populate cmd1
	while(strcmp(args[i], "|") !=  0) {
		cmd1[i] = args[i];
		i++;
	} 
	cmd1[i] = NULL;
	i++;
	
	unsigned l = 0;
	// populate cmd2
	while(args[i] != NULL) {
		cmd2[l] = args[i];
		i++;
		l++;
	}
	cmd2[l] = NULL;
		 

	// create pipe fle descriptor and call pipe
	int pipe_fd[2];
	pipe(pipe_fd);

	// handle piping
	run_pipe(pipe_fd, cmd1, cmd2, bg);
}

// method to handle piping
void run_pipe(int *pipe_fd, char **cmd1, char **cmd2, int bg) {
	
	// fork process to pipe
	pid_t cpid1 = fork();

	// forking failure
	if(cpid1 == -1) {
		perror("fork");
		exit(1);
	}

	// child
	else if(cpid1 == 0) {
		
		// output	
		close(pipe_fd[0]);
		dup2(pipe_fd[1], 1);

		execvp(cmd1[0], cmd1);
		
		perror(cmd1[0]);
		
	}

	// parent
	else {
		// fork for writing
		int cpid2 = fork();

		// child
		if(cpid2 == 0) {
			
			// input
			close(pipe_fd[1]);
			dup2(pipe_fd[0], 0);

			execvp(cmd2[0], cmd2);
			
			perror(cmd2[0]);
		}
		
		// close both ends of the pipe
		close(pipe_fd[0]);
		close(pipe_fd[1]);
		
		// background
		// check if child has returned quickly
		// don't block if still running 
		if(bg) {
			waitpid(cpid1, NULL, WNOHANG);
			waitpid(cpid2, NULL, WNOHANG);
		}	
		
		// wait until child really finishes 
		else {
		waitpid(cpid1, NULL, 0);
		waitpid(cpid2, NULL, 0);
		}
	}
}
