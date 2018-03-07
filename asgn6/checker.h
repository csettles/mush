#ifndef CHECKER_H
#define CHECKER_H

#include "parseline.h"

typedef struct stage stage;
struct stage {
	int num, argc;
	char input[FILE_LEN], output[FILE_LEN];
	char command[CMD_LEN];
	char line[LINE_MAX];
	char args[ARG_MAX][ARG_LEN]; /* list of null terminated arguments */
	stage *next;
};

void handle_stage(stage *s, char *input, int stage_max);

void handle_input(stage *s, char *input, int stage_max);
void handle_output(stage *s, char *input, int stage_max);
void handle_args(stage *s, char *input, int stage_max);

stage *new_stage(int number);
stage new_stage_s(int number);
stage *build_stages(char stages[STAGE_MAX][LINE_MAX], int len);
int num_stages(stage *s);
void print_stage(stage s);
void print_stages(stage head);

#endif
