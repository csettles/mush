/*=============================================================================
 *   Assignment:  Assignment 6: mush
 *
 *       Author:  Caitlin Settles and Donald Loveland
 *        Class:  CSC 357 Section 01
 *     Due Date:  03/14/18
 *
 *-----------------------------------------------------------------------------
 *
 *  Description:  Miniminall Useful SHell
 *
 *        Input:  A pipeline of up to 10 commands
 *
 *===========================================================================*/

#include "mush.h"

int main(int argc, const char * argv[]) {
	char line[LINE_MAX + 2];
	sigset_t new, old;
	int i;
	FILE* fp;
	int content;
	
	sigemptyset(&new);
	sigaddset(&new, SIGINT);
	sigprocmask(SIG_BLOCK, &new, &old);
	
	/* sigint is now disabled */
	
	if (argc > 1) {
		/* read file and get_stages for each line */
		fp = fopen(argv[1], "r");
		if (fp == NULL) {
			perror(argv[1]);
			exit(EXIT_FAILURE);
		}
		
		i = 0;
		memset(line, 0, strlen(line));
		
		while((content = fgetc(fp)) != EOF) {
			if (i >= LINE_MAX) {
				/* this line is too big */
				/* if new line char, wipe it */
				if (content == '\n') {
					i = 0;
					memset(line, 0, strlen(line));
					fprintf(stderr, "command too long\n");
					continue;
				}
				/* let it keep running, eating up line */
				i--;
			} else {
				if (content == '\n') {
					/* will have grabbed line, set null */
					line[i] = 0;
					
					eval_pipeline(line, old);
					
					/* cleans out line */
					i = 0;
					memset(line, 0, strlen(line));
				} else {
					line[i++] = content;
				}
			}
		}
		fclose(fp);
		return 0;
	} else {
		while (1) {
			/* drop out of this loop with EOF or SIGQUIT */
			prompt(line);
			eval_pipeline(line, old);
		}
		return 0;
	}
}

/**
 Evaluates a pipeline (series of commands) by forking and exec'ing each
 stage.

 @param line the lineo of commands
 @param old the old proc mask to return to in the children
 */
void eval_pipeline(char *line, sigset_t old) {
	stage *s;
	int i, num_pipes, status, signaled = 0;
	int fds[STAGE_MAX * 2];
	pid_t proc;
	
	
	if ((s = get_stages(line)) == NULL) {
		/* line was too long error, otherwise other error */
		return;
	}
	
	num_pipes = num_stages(s) - 1;
	for (i = 0; i < num_pipes; i++) {
		if (pipe(fds + i * 2)) {
			perror("mush");
			exit(EXIT_FAILURE);
		}
	}
	
	for (i = 0; i < num_pipes + 1; i++) {
		if ((proc = fork()) == 0) {
			sigprocmask(SIG_SETMASK, &old, NULL);
			exec_command(fds, num_pipes, s);
		} else if (proc < 0) {
			perror("mush");
			exit(EXIT_FAILURE);
		}
		
		fflush(stdout);
		s = s->next;
	}
	
	for (i = 0; i < num_pipes * 2; i++) {
		/* close open pipes so processes don't hang */
		close(fds[i]);
	}
	
	for (i = 0; i < num_pipes + 1; i++) {
		proc = wait(&status);
		if (WIFSIGNALED(status) && WTERMSIG(status) == 2) {
			/* prints newline if ^C is pressed */
			signaled = 1;
		}
	}
	
	if (signaled) {
		printf("\n");
	}
}

/**
 If the shell is run in interactive mode, prints the prompt and does some
 preliminary processing on the input. Checks the user hasn't sent an EOF and
 replaces the newline with a null character.

 @param line the buffer to store the line in
 */
void prompt(char *line) {
	int ind;
	
	printf("8-P ");
	fgets(line, LINE_MAX + 2, stdin);
	if (feof(stdin)) {
		printf("\n");
		exit(0);
	}
	
	ind = (int)strlen(line) - 1;
	if (line[ind] == '\n') {
		line[ind] = '\0';
	}
}

/**
 Returns a stage as long as there are no other errors. An error case can be
 the input is over the maximum length allowed, or the command is nonexistant
 (empty). Also catches the case of cd or exit, in which case the action is
 performed before further processing of the line.

 @param line the line of input
 @return NULL or a pointer to a list of stages
 */
stage *get_stages(char *line) {
	int str_len, stage_len;
	int c;
	char *dir;
	char stages[STAGE_MAX][LINE_MAX];
	
	str_len = (int)strlen(line);
	if (str_len > LINE_MAX) {
		while ((c = getchar()) != '\n' && c != EOF) {
			/* flush stdin */;
		}
		fprintf(stderr, "command too long\n");
		return NULL;
	} else if (str_len == 0) {
		return NULL;
	}
	
	stage_len = split_line(line, stages);
	if (stage_len < 0) {
		return NULL;
	}
	
	if (clean_line(line, stages, stage_len) == 1) {
		return NULL;
	}
	
	if (strstr(line, "cd") != NULL) {
		dir = strtok(line, " ");
		dir = strtok(NULL, " ");
		if (all_space(dir)) {
			fprintf(stderr, "destination required\n");
			return NULL;
		}
		if (chdir(dir)) {
			perror(dir);
		}
		return NULL;
	}
	
	if (strcmp(line, "exit") == 0) {
		exit(EXIT_SUCCESS);
	}
	
	return build_stages(stages, stage_len);;
}

void exec_command(int fds[20], int ind_max, stage *s) {
	char **args;
	int i, tmp_in, tmp_out;
	
	if (ind_max > 0) {
		if (s->num == 0) {
			dup2(fds[1], STDOUT_FILENO);
			if (strcmp(s->input, "original stdin") != 0) {
				if ((tmp_in = open(s->input, O_RDONLY)) < 0) {
					perror(s->input);
					exit(EXIT_FAILURE);
				}
				dup2(tmp_in, STDIN_FILENO);
			}
		} else if (s->num == ind_max) {
			dup2(fds[s->num * 2 - 2], STDIN_FILENO);
			if (strcmp(s->output, "original stdout") != 0) {
				if ((tmp_out = open(s->output,
						    O_WRONLY | O_CREAT | O_TRUNC,
						    0666)) < 0) {
					perror(s->output);
					exit(EXIT_FAILURE);
				}
				dup2(tmp_out, STDOUT_FILENO);
			}
		} else {
			dup2(fds[s->num * 2 - 2], STDIN_FILENO);
			dup2(fds[s->num * 2 + 1], STDOUT_FILENO);
		}
	} else {
		if (strcmp(s->input, "original stdin") != 0) {
			if ((tmp_in = open(s->input, O_RDONLY)) < 0) {
				perror(s->input);
				exit(EXIT_FAILURE);
			}
			dup2(tmp_in, STDIN_FILENO);
		}
		
		if (strcmp(s->output, "original stdout") != 0) {
			if ((tmp_out = open(s->output,
					    O_WRONLY | O_CREAT | O_TRUNC,
					    0666)) < 0) {
				perror(s->output);
				exit(EXIT_FAILURE);
			}
			dup2(tmp_out, STDOUT_FILENO);
		}
	}
	
	for (i = 0; i < ind_max * 2; i++) {
		close(fds[i]);
	}
	
	args = malloc(sizeof(char *) * (s->argc + 1));
	for (i = 0; i < s->argc; i++) {
		args[i] = malloc(sizeof(char) * strlen(s->args[i]));
		strcpy(args[i], s->args[i]);
	}
	args[s->argc] = NULL;
	
	if (execvp(s->args[0], args)) {
		perror(s->args[0]);
		exit(EXIT_FAILURE);
	}
}
