#include "checker.h"

/**
 Creates a new stage pointer.

 @param number the stage number
 @return the newly created stage pointer
 */
stage *new_stage(int number) {
	stage *s;
	s = malloc(sizeof(struct stage));
	s->next = NULL;
	s->num = number;
	s->argc = 0;

	return s;
}

/**
 Creates a new stage struct.

 @param number the stage number
 @return the newly created stage struct
 */
stage new_stage_s(int number) {
	stage s;
	s.num = number;
	s.argc = 0;
	s.next = NULL;
	return s;
}

/**
 Populates a stage from a "stage" of input.

 @param s the stage to populate
 @param input the command, args, and redirections
 @param stage_max the index of the last stage
 */
void handle_stage(stage *s, char *input, int stage_max) {
//	int last_index;
	
//	last_index = (int)strlen(input) - 1;
//	if (input[last_index] == '\n') {
//		input[last_index] = '\0';
//		last_index--;
//	}

	if (strlen(input) == 0 || all_space(input)) {
		fprintf(stderr, "invalid null command\n");
		exit(EXIT_FAILURE);
	}
	
	/* input is now the command */
	
	strcpy(s->line, input);
	
	handle_input(s, input, stage_max);
	handle_output(s, input, stage_max);
	handle_args(s, input, stage_max);
}

/**
 Determines where the input to the stage is coming from.

 @param s the stage
 @param input the string of the command, args, and redirections
 @param stage_max the index of the last stage
 */
void handle_input(stage *s, char *input, int stage_max) {
	char *redir_pos, *input_dest;
	char position_temp[LINE_MAX];
	
	strcpy(position_temp, input);
	
	if (s->num == 0) {
		/* Find the redirection */
		redir_pos = strchr(position_temp, '<');
		/* Couldn't find redirection */
		if (redir_pos == NULL) {
			strncpy(s->input, "original stdin", FILE_LEN);
		} else {
			/* Moves to space, then moves to word */
			input_dest = strtok(redir_pos, " ");
			input_dest = strtok(NULL, " ");
			/* if extra white space, moves past */
			while(*input_dest == ' ') {
				input_dest = strtok(NULL, " ");
			}
			strncpy(s->input, input_dest, FILE_LEN);
		}
		/* Interior of pipeline now only have to consider next pipe */
	} else {
		sprintf(s->input, "pipe from stage %d", s->num - 1);
	}
}

/**
 Determines where the stage will give its output.

 @param s the stage
 @param input the string of the command, args, and redirections
 @param stage_max the index of the last stage
 */
void handle_output(stage *s, char *input, int stage_max) {
	char *redir_pos, *output;
	char position_temp[LINE_MAX];
	
	strcpy(position_temp, input);
	
	/* if last stage or if only one stage */
	if (s->num == stage_max) {
		/* Find the redirection */
		redir_pos = strchr(position_temp, '>');
		/* Couldn't find redirection */
		if (redir_pos == NULL) {
			strncpy(s->output, "original stdout", FILE_LEN);
		} else {
			/* Moves to space, then moves to word */
			output = strtok(redir_pos, " ");
			output = strtok(NULL, " ");
			/* if extra white space, moves past */
			while(*output == ' ') {
				output = strtok(NULL, " ");
			}
			strncpy(s->output, output, FILE_LEN);
		}
		/* Interior of pipeline now only have to consider next pipe */
	} else {
		sprintf(s->output, "pipe to stage %d", s->num + 1);
	}
}

/**
 Determines the command and arguments to the stage, as well as the number of
 arguments it has.

 @param s the stage
 @param input the string of the command, arguments, and redirections
 @param stage_max the index of the last stage
 */
void handle_args(stage *s, char *input, int stage_max) {
	char input_copy[LINE_MAX];
	char *token;
	int len = 0;
	
	strcpy(input_copy, input);
	
	token = strtok(input_copy, " ");
	strncpy(s->command, token, CMD_LEN);
	while (token != NULL && len < ARG_MAX) {
		if ((strcmp(token,"<") == 0) || (strcmp(token, ">") == 0)) {
			token = strtok(NULL, " "); /* skip file name */
			token = strtok(NULL, " "); /* go to next arg */
			continue;
		}
		if (strlen(token) >= ARG_LEN) {
			fprintf(stderr, "argument name too long\n");
			exit(EXIT_FAILURE);
		}
		strncpy(s->args[len], token, ARG_LEN);
		len++;
		token = strtok(NULL, " ");
	}
	*s->args[len] = (char)NULL;
	
	if (len == ARG_MAX && token != NULL) {
		fprintf(stderr, "too many arguments\n");
		exit(EXIT_FAILURE);
	}
	
	s->argc = len;
}

/**
 Builds a linked list of stages from a line of input.

 @param stages each stage represented as a string
 @param len the number of stages in the line
 @return stages in the input as represented by structs
 */
stage *build_stages(char stages[STAGE_MAX][LINE_MAX], int len) {
	int i;
	stage *s, *temp;
	
	s = new_stage(0);
	handle_stage(s, stages[0], len - 1);
	
	temp = s;
	for (i = 1; i < len; i++) {
		temp->next = new_stage(i);
		handle_stage(temp->next, stages[i], len - 1);
		temp = temp->next;
	}
	
	return s;
}

/**
 Gets the number of stages in a pipeline.

 @param s the head of the list of stages
 @return the number of stages in the pipeline
 */
int num_stages(stage *s) {
	if (s == NULL) {
		return 0;
	}
	
	return 1 + num_stages(s->next);
}

/**
 Prints a single stage.

 @param s the stage
 */
void print_stage(stage s) {
	int i;
	
	printf("\n--------\n");
	printf("Stage %d: \"%s\"\n", s.num, s.line);
	printf("--------\n");
	
	printf("%12s", "input: ");
	printf("%s\n", s.input);
	
	printf("%12s", "output: ");
	printf("%s\n", s.output);
	
	printf("%12s", "argc: ");
	printf("%d\n", s.argc);
	
	printf("%12s", "argv: ");
	for (i = 0; i < s.argc - 1; i++) {
		printf("\"%s\",", s.args[i]);
	}
	printf("\"%s\"\n", s.args[i]);
}

/**
 Prints a list of stages.

 @param head the first stage in the list
 */
void print_stages(stage head) {
	print_stage(head);
	
	while (head.next != NULL) {
		print_stage(*head.next);
		head = *head.next;
	}
}


