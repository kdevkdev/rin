/*
Author: davex
e-mail: efengeler@gmail.com
*/

#include "inc/rewind.h"

static int num_rwnd_states = 0;
int rwnd_state_size = 0;

struct rewind_state{
	int have_data;
	unsigned char *data;
	struct rewind_state *next;
	struct rewind_state *prev;
};


struct rewind_state *ptr_rewind_states, *prev_state, *next_state;


int allocate_rewind_states(void){

	if(num_rwnd_states != 0)
		return 0;

	struct rewind_state *created_state, *first_state;
	int i;
	rwnd_state_size = gb_save_state(NULL);  //declared in "gb/gb.h"
	num_rwnd_states = (int) ( (float)TOTAL_REWIND_MEMORY / (float) rwnd_state_size );
	
	//reserves first state
	created_state =  (struct rewind_state *)malloc( sizeof(struct rewind_state) );
	created_state->have_data = 0;
	created_state->data = (byte *) malloc( rwnd_state_size);
	first_state = created_state;
	prev_state = first_state;
	
	//reserves remaining states
	for( i = 1; i< num_rwnd_states; i++){
		created_state  = (struct rewind_state *)malloc( sizeof(struct rewind_state) );
		created_state->have_data = 0;
		created_state->data = (byte *) malloc( rwnd_state_size);
		created_state ->prev = prev_state;
		prev_state->next = created_state;
		prev_state = created_state;
	}
	
	
	//make list be circular
	created_state->next = first_state; 
	first_state->prev = created_state; 
	ptr_rewind_states = first_state;
	return 1;
}


int free_rewind_states(void){
	
	if(num_rwnd_states == 0)
		return 0;	

	struct rewind_state *now_state; 
	
	now_state = ptr_rewind_states;
	prev_state = now_state->prev;
	prev_state->next = NULL;
	
	while(now_state != NULL){
		//if ( now_state == NULL)
		//	break;
		next_state = now_state->next;
		free(now_state->data );
		free(now_state);
		now_state = next_state;
	}

	num_rwnd_states = 0;
	rwnd_state_size = 0;

	ptr_rewind_states = NULL;
	prev_state = NULL;
	next_state = NULL;

	return 1;
}


int save_rewind_state(void){
	
	if(num_rwnd_states == 0)
		return 0;	

	gb_save_state(ptr_rewind_states->data); //declared in "gb/gb.h"
	ptr_rewind_states->have_data = 1;
	ptr_rewind_states = ptr_rewind_states->next;
	return 1;	
}

int read_rewind_state(void){

	if(num_rwnd_states == 0)
		return 0;		

	int ret_val = -999;
	prev_state = ptr_rewind_states->prev;

	if (prev_state->have_data > 0 ){
		load_state_tmp( prev_state->data); 
		prev_state->have_data = 0;
		ptr_rewind_states = ptr_rewind_states->prev;
		ret_val = 1;
	}
	return ret_val;
}

