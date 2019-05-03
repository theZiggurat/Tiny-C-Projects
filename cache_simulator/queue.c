#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#define FIFO 0
#define LRU 1

struct queue{
	int sets;
	int assoc;
	int replacement_pol;
};

struct queue *queue_(int set_size, int assoc, int policy){
	struct queue *ret;
	ret =(struct queue *) malloc (sizeof(struct queue *));
	ret->sets = set_size;
	ret->assoc = assoc;
	return ret;
}

// returns address of least recently used or first in if queue is full
// if not full, returns 0
uint64_t bump(struct queue *q, uint64_t q_data[], int si, uint64_t address_add){
	
	int i;
	
	uint64_t ret;
	ret = q_data[(si*q->assoc)+q->assoc-1];
	
	for(i = q->assoc-1; i>0; i--){
		q_data[(si*q->assoc)+i] = q_data[(si*q->assoc)+i-1];
	}
	q_data[(si*q->assoc)] = address_add;
	
	return ret;
}

// only to be used when cache is LRU so that cache hits can change priority of
// address in queue
void moveToFront(struct queue *q, uint64_t q_data[], int si, uint64_t address_move){
	int i, k;
	for(i = 0; i<q->assoc; i++){
		
		if(q_data[(si*q->assoc)+i] == address_move){
			
			for(k = i; k>0; k--){
				q_data[(si*q->assoc)+k] = q_data[(si*q->assoc)+k-1];
			}
			
			q_data[si*q->assoc] = address_move;
			break;
		}
	}
}

void print_queue(struct queue *q, uint64_t q_data[]){
	int i, k;
	for(i = 0; i<q->sets; i++){
		printf(" ------- SET %i --------\n", i);
		for(k = 0; k<q->assoc; k++){
			printf("%" PRIu64 " ",q_data[(i*q->assoc)+k]);
		}
		printf("\n");
	}
	
}
