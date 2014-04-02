#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>

struct garbage_item
{
	void* victim;
	unsigned long long version;
	struct garbage_item *next;
};

struct garbage_item* add_garbage(void *victim);

void empty_garbage();
void stop_gc();
