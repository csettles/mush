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
				if (pipe(fds + i * 2) < 0) {
					perror("mush");
					exit(EXIT_FAILURE);
				}
			}
			
			while (s) {
				/* do stuff */
				if ((child = fork()) == 0) {
					sigprocmask(SIG_SETMASK, &old, NULL);
					exec_command(fds, s->num, s);
				} else {
					waitpid(child, NULL, 0);
				}
				/* fork */
				fflush(stdout);
				return 0;
			}
		}
	}
}

void prompt(char *line) {
	printf("mush%% ");
	fgets(line, LINE_MAX + 2, stdin); /* check feof here */
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
		chdir(dir + 1);
		return NULL;
	}
	
	s = build_stages(stages, stage_len);
	
	return s;
}

void exec_command(int fds[20], int ind, stage *s) {
	printf("process %d", ind);
	
	if (strcmp(s->input, "original stdin") == 0) {
		/* stdin */
		dup2(<#int#>, <#int#>);
	} else if (strstr(s->input, "pipe from stage")) {
		/* pipe from ind - 1 */
	} else {
		/* file */
	}
}
