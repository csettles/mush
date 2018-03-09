//
//  main.c
//  asgn6
//
//  Created by Caitlin Settles on 3/7/18.
//  Copyright © 2018 Caitlin Settles. All rights reserved.
//

#include "mush.h"

int main(int argc, const char * argv[]) {
	char line[LINE_MAX + 2];
	sigset_t new, old;
	stage *s;
	int fds[20], num_pipes, i;
	pid_t child;
	
	sigemptyset(&new);
	sigaddset(&new, SIGINT);
	sigprocmask(SIG_BLOCK, &new, &old);
	
	/* sigint is now disabled */
	
	if (argc > 1) {
		/* read file and get_stages for each line */
		return 0;
	} else {
		while (1) {
			/* only way to drop out of this loop is ^D */
			
			prompt(line);
			if ((s = get_stages(line)) == NULL) {
				/* directory was changed */
				continue;
			}
			
			num_pipes = num_stages(s) - 1;
			for (i = 0; i < num_pipes; i++) {
				if (pipe(fds + i * 2)) {
					perror("mush");
					exit(EXIT_FAILURE);
				}
			}
			
			while (s) {
				if (fork() == 0) {
					sigprocmask(SIG_SETMASK, &old, NULL);
					exec_command(fds, num_pipes, s);
				}
				
				fflush(stdout);
				s = s->next;
			}
			
			for (i = 0; i < num_pipes * 2; i++) {
				/* close all open files so processes don't hang */
				close(fds[i]);
			}
			
			for (i = 0; i < num_pipes + 1; i++) {
				child = wait(NULL);
				printf("child %d exited\n", child);
			}
		}
		return 0;
	}
}

void prompt(char *line) {
	int ind;
	
	printf("mush>\t");
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

stage *get_stages(char *line) {
	int str_len, stage_len;
	char *dir;
	char stages[STAGE_MAX][LINE_MAX];
	stage *s;
	
	str_len = (int)strlen(line);
	if (str_len > LINE_MAX) {
		/* here we need to consume rest of line if it is too long */
		fprintf(stderr, "command too long\n");
		exit(EXIT_FAILURE);
	}
	
	stage_len = split_line(line, stages);
	clean_line(line, stages, stage_len);
	
	if (strstr(line, "cd") != NULL) {
		/* not tested yet */
		dir = strtok(line, " ");
		dir = strtok(NULL, " ");
		if (all_space(dir)) {
			fprintf(stderr, "need a directory, dummy\n");
			return NULL;
		}
		if (chdir(dir)) {
			perror(dir);
		}
		return NULL;
	}
	
	s = build_stages(stages, stage_len);
	
	return s;
}

void exec_command(int fds[20], int ind_max, stage *s) {
	char **args;
	int i;
	
	if (ind_max > 0) {
		if (s->num == 0) {
			dup2(fds[1], 1); /* stdout */
		} else if (s->num == ind_max) {
			dup2(fds[s->num * 2 - 2], 0); /* stdin */
		} else {
			dup2(fds[s->num * 2 - 2], 0); /* stdin */
			dup2(fds[s->num * 2 + 1], 1); /* stdout */
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
	
	execvp(s->args[0], args);
}
