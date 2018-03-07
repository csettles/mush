/**
  parseline.h
  asgn5

  Created by Caitlin Settles on 3/3/18.
  Copyright © 2018 Caitlin Settles. All rights reserved.
**/

#ifndef parseline_h
#define parseline_h

#define STAGE_MAX 10
#define LINE_MAX 512
#define ARG_MAX 10
#define CMD_LEN 20
#define FILE_LEN 20
#define ARG_LEN 20

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int split_line(char *line, char stages[STAGE_MAX][LINE_MAX]);
void clean_line(char *line, char stages[STAGE_MAX][LINE_MAX], int len);
int all_space(char *line);

#endif /* parseline_h */
