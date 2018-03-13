/*
//  main.c
//  asgn6
//
//  Created by Caitlin Settles on 3/7/18.
//  Copyright © 2018 Caitlin Settles. All rights reserved.
//
*/

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
			}
			if (content == '\n') {
				/* will have grabbed line, set null term */
				line[i] = 0; 
				
				eval_pipeline(line, old);

				/* cleans out line */
				i = 0;
				memset(line, 0, strlen(line)); 
			} else {	
				line[i++] = content;
			}
		}	
		fclose(fp);  		
		return 0;
	} else {
		while (1) {
			/* only way to drop out of this loop is ^D */
			prompt(line);
			eval_pipeline(line, old);
		}
		return 0;
	}
}

void eval_pipeline(char *line, sigset_t old) {
	stage *s;
	int i, num_pipes, status;
	int fds[STAGE_MAX * 2];
	pid_t proc, children[STAGE_MAX];
	
	
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
		if ((children[i] = fork()) == 0) {
			sigprocmask(SIG_SETMASK, &old, NULL);
			exec_command(fds, num_pipes, s);
		} else if (children[i] < 0) {
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
		/* exits cleanly */
		if (WEXITSTATUS(status) != 0) {
			printf("child %d failed\n", proc);
			kill(-getpgrp(), SIGINT);
		}
	}
}

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
