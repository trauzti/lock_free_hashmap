#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include "gc.h"

struct garbage_item *garbage_head = 0;
struct garbage_item *garbage_curr = 0;
struct garbage_item *candidate_head = 0;
struct garbage_item *candidate_curr = 0;

pthread_mutex_t swap_mutex;
int stop=0;

struct garbage_item* add_garbage(void *victim)
{
	struct garbage_item *ptr;
	ptr = (struct garbage_item *)malloc(sizeof(struct garbage_item));	
	if(NULL == ptr)
	{
		printf("\n Node creation failed \n");
		return 0;
	}
	ptr->victim = victim;
	ptr->next = 0;

	

	
	if(candidate_head == 0)
	{
		struct garbage_item *my_candidate_curr
		__sync_bool_compare_and_swap(&candidate_head,0,ptr);
		bool exchanged = __sync_bool_compare_and_swap(&candidate_curr,0,ptr);
		if (!exchanged) {
			my_candidate_curr=candidate_curr;
			while (1) {
				exchanged = __sync_bool_compare_and_swap(&candidate_curr,my_candidate_curr,ptr);
				if (exchanged) 
					break;
				else
					my_candidate_curr = candidate_curr;
			}
			my_candidate_curr->next = ptr;
		}
	}	
	else
	{
		struct garbage_item *my_candidate_curr = candidate_curr;
		while (1) {
			bool exchanged = __sync_bool_compare_and_swap(&candidate_curr,my_candidate_curr,ptr);
			if (exchanged) 
				break;
			else
				my_candidate_curr = candidate_curr;
		}
		my_candidate_curr->next = ptr;
	}
	
	return candidate_curr;
}

// Swaps the garbage list and the candidate list
void swap_lists()
{
	struct garbage_item *tmp = 0;

	tmp = candidate_head;
	candidate_head = garbage_head;
	garbage_head = tmp;

	tmp = candidate_curr;
	candidate_curr = garbage_curr;
	garbage_curr = tmp;
}

// empties the garbage list
void empty_garbage()
{
	struct garbage_item *ptr;
	struct garbage_item *tmp;

	unsigned long long min_version = 0;

	struct timespec tim, tim2;
	tim.tv_sec = 0;
	tim.tv_nsec = 100000000L;

	while (!stop) {
		ptr = 0;
		tmp = 0;

		// Sleep 
		nanosleep(&tim , &tim2);

		if (stop) break;	

		// get the lock	
		//pthread_mutex_lock(&swap_mutex);

		int worker_id;
		ptr = garbage_head;

		// empty the garbage list
		while (ptr != 0)
		{
			if (ptr->next==0) {
				garbage_curr=ptr;
			}

			free(ptr->victim);
			ptr->victim = 0;

			tmp = ptr;
			ptr = ptr->next;

			// Move the head pointer forward if the first item is deleted
			if (garbage_head==tmp) {
				garbage_head=ptr;
			}
			// free the linkedList object
			free(tmp);
			tmp = 0;
		}
		// If the list is empty
		if (garbage_head==0) {	
			garbage_curr = 0;
		}

		swap_lists();

		// release the lock
		//pthread_mutex_unlock(&swap_mutex);
	}
}

void stop_gc()
{
	stop=1;
}

