//
//  main.c
//  asgn6
//
//  Created by Caitlin Settles on 3/7/18.
//  Copyright © 2018 Caitlin Settles. All rights reserved.
//

#include "mush.h"

int main(int argc, const char * argv[]) {
	stage *s;
	
	if (argc > 1) {
		/* open file and use that as stdin */
		return 0;
	} else {
		s = show_prompt();
		while (s) {
			/* do stuff */
			return 0;
		}
	}
}

stage *show_prompt(void) {
	char line[LINE_MAX + 2];
	int str_len;
	stage *s;
	
	printf("mush%% ");
	fgets(line, LINE_MAX + 2, stdin);
	
	str_len = (int)strlen(line);
	if (str_len > LINE_MAX) {
		/* here we need to consume rest of line if it is too long */
		fprintf(stderr, "command too long\n");
		exit(EXIT_FAILURE);
	}
	
	s = build_stages(line);
	
	return s;
}
