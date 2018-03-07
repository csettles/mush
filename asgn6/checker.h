#ifndef CHECKER_H
#define CHECKER_H

#define STAGE_MAX 10
#define LINE_MAX 512
#define ARG_MAX 10
#define CMD_LEN 20
#define FILE_LEN 20
#define ARG_LEN 20

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
stage *build_stages(char *line);
void print_stage(stage s);
void print_stages(stage head);

#endif
