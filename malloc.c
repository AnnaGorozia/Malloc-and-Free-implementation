#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>

struct block{
	struct block* prev;
	struct block* next;
	size_t size;
	int available;
};

struct block* first_block;
int first_call = 0;
struct block* last_block;

/*
 * This function is called every time malloc is called,
 * Except it is the first call.
 * In this function free space is searched using worst fit
 * Algorithm. This function goes through linked list and searches
 * for the largest free memory block. After the loop, if found 
 * block's size is enough for user, program returns it, otherwise
 * NULL is returned
 * Also function checks, if user will need all the memory from the
 * found memory block. 
 * If user needs less, than new memory block is added to the linked list
 * Which has size of found maximum size minus the block size and the size
 * User asks for.
 */

struct block* search_free_block(size_t size){
	struct block* found = NULL;
	struct block* cur;
	size_t max_size = first_block->size;
	
	for(cur = first_block; cur != NULL; cur = cur->next){
		if(cur->available == 1){
			continue;
		} 
		if(found == NULL || cur->size > max_size){
			max_size = cur->size;
			found = cur;
		}
	}
	
	if(max_size < size){
		return NULL;
	}else{
		int unused_size = max_size - (sizeof(struct block) + size);
		if(unused_size > 0){
			struct block* new_block = found + size;
			new_block->size = unused_size;
			new_block->available = 0;
			new_block->next = found->next;
			new_block->prev = found;
			found->next = new_block;
		}
		return found;
	}
}

/* This function is called after every malloc called
 * If linked list is not initialized, in this function 
 * Is created first block of list and 0 is returned
 * Otherwise it returns 1
 */
int initialize(size_t size){
	if(first_call == 1) return 1;
	/*
	 * First and last block are same for the initial state
	 * Program's data space is incremented by size of block itself
	 * Plus size user wants to use
	 */
	first_call = 1;
	first_block = (struct block*)sbrk(sizeof(struct block) + size);
	first_block->size = size;
	first_block->available = 1;
	first_block->next = NULL;
	first_block->prev = NULL;
	last_block = first_block;
	
	return 0;
}

/*
 * This function is called by free function
 * Passed arguments are current and next block
 * function checks if both blocks are not NULL
 * And if both are available. In this case it 
 * Concatenates these blocks and next block
 * Disappears and current block becomes bigger available
 * block in memory linked list
 */

void block_union(struct block* cur, struct block* next){
	if(next == NULL || cur == NULL) return;
	if(next->available == 1 || cur->available == 1) return;
	size_t new_size = next->size + cur->size + sizeof(struct block);
	cur->size = new_size;
	cur->next = next->next;
}

/*
 * Free function firstly makes union of memory blocks 
 * For current block (looks at previous and next blocks)
 * Then it goes through the linked list and checks if there are
 * Any neighbour blocks, which are both available and in this case
 * calls the union function, which merges current and next block
 */
void free(void* ptr){
	struct block *b = (struct block*)(ptr);  
	b->available = 0;  
	block_union(b, b->next);
	block_union(b->prev, b);
	struct block * curr;
	for(curr = first_block; curr != NULL; curr = curr->next){
		block_union(curr, curr->next);
	}
}

/*
 * Malloc function first checks, if it is the first call
 * If it is, than it calls initialize function, which initializes
 * Linked list and created first and last blocks and the first 
 * Block is Returned.
 * Otherwise function calls search_free_block function.
 * If returned pointer is not NULL function writes in returned block
 * The size user asked for and checks that block is used and returns
 * Found block.
 * If program could not find free block in linked list, than new block 
 * Is created. New block is added to the linked list in the end, after
 * Last block and this new block now becomes the last block and it is returned
 */
void* malloc(size_t size){
	if(initialize(size) == 0){
		return first_block;
	} 
	
	struct block* res = search_free_block(size);
	
	if(res != NULL){ 
		res->size = size;
		res->available = 1;
		return res;
	}
	
	struct block* new_block = sbrk(size+sizeof(struct block));
	
	new_block->size = size;
	new_block->available = 1;
	new_block->next = NULL;
	new_block->prev = last_block;
	last_block->next = new_block;
	last_block = new_block;
	
	return new_block;
	
}

/*
 * This function goes through the linked list and prints out
 * Pointer to the block, size in this block and is block available
 * For usage or not
 */
 void meminfo(){
	struct block * curr;
	for(curr = first_block; curr != NULL; curr = curr->next){
		printf("Pointer: %p ", curr);
		printf("Size: %i ", curr->size);
		if(curr->available == 0)
			printf("Is available\n");
		else
			printf("Is not available\n");
	}
	printf("----------------\n");
}

int main(int argc, const char *argv[]){ 
 	void * a = malloc(4);
	void * b = malloc(128);
 	void * c = malloc(36);
	meminfo();
	free(a);
 	free(b);
	free(c);
  	meminfo();
 	void * e = malloc(16);
 	void * f = malloc(4);
 	void * g = malloc(4);
	meminfo();
	free(e);
 	free(f);
	free(g);
	meminfo();
}



