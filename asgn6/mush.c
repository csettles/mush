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
			/* fork and exec processes */
		}
	}
}
