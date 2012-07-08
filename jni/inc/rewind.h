#ifndef REWIND_H
#define REWIND_H

/*
Author: davex
e-mail: efengeler@gmail.com
*/
//some little changes made

#include "main.h"

#define TOTAL_REWIND_MEMORY  10*1024*1024 //reserves 10 MB for rewind states

//returs 0 if already allocated, 1 otherwise
int allocate_rewind_states(void);

//all return 0 if none allocated, 1 otherwise
int read_rewind_state(void);
int save_rewind_state(void);
int free_rewind_states(void);


#endif
