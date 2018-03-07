/**
    mush.h
    asgn6
 
    Created by Caitlin Settles on 3/7/18.
    Copyright © 2018 Caitlin Settles. All rights reserved.
**/

#ifndef mush_h
#define mush_h

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "checker.h"

void prompt(char *line);
stage *get_stages(char *line);
void exec_command(int fds[20], int ind, stage *s);

#endif /* mush_h */
