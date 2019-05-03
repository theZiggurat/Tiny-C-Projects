#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include "queue.c"

#define ASSOC 0
#define DIRECT 1
#define ASSOCN 2

#define FIFO 0
#define LRU 1

int sets, offsetBits, setIndexBits; // global bit size for easy offsetting addresses

int powerOf2(int x);
int getSetIndex(long long mem_address);
uint64_t getTag(long long mem_address);
bool compareTag(long long mem_arg_1, long long mem_arg_2);
void printCache(uint64_t data[], int size);

int main(int argc, char **argv){
	
	int MEM_R = 0, MEM_W = 0, CACHE_H = 0, CACHE_M = 0;	// 	    NO PRE-FETCH COUNTERS
	int MEM_R_P = 0, MEM_W_P = 0, CACHE_H_P = 0, CACHE_M_P = 0; // PRE-FETCH COUNTERS
	
	int rep_pol;				// replacement policy, defined as FIFO=1 and LRU=0
	int assoc_type;				// association type, defined as ASSOC=0, DIRECT=1, ASSOCN=2
	
	char counter[16]; // buffer for counter 
	char address[16]; // buffer for address
	char instruction; // buffer for R or W
	int cacheSize, // holds cache size argument
	blockSize, // holds block size argument
	assoc; // if n way associated, holds n
	
	
	FILE *f;
	f = fopen(argv[5], "r");
	
	// checks if file path is wrong
	if(f==NULL){
		printf("Error, invalid path\n");
		return 1;
	}
	
	// checking if blocksize and cachesize are valid
	cacheSize = atoi(argv[1]);
	blockSize = atoi(argv[4]);
	offsetBits = powerOf2(blockSize);
	if(powerOf2(cacheSize)==-1){ printf("Error, cache size is not a power of 2!\n");
		return 1;}
	if(blockSize == -1 ){ printf("Error, block size is not a power of 2!\n");
		return 1;}
	
	// associativity
	if(strcmp(argv[2], "direct")==0){
		assoc_type = DIRECT;
		sets = cacheSize / blockSize;
		assoc = 1;
	}
	else if(strcmp(argv[2], "assoc")==0){
		assoc_type = ASSOC;
		sets = 1;
		assoc = cacheSize / blockSize;
		
	}
	else if(strcmp(argv[2], "assoc:n")==-1){
		assoc_type = ASSOCN;
		sscanf(argv[2], "assoc:%i", &assoc);
		if(!powerOf2(assoc)&&assoc!=1){ printf("Error, cache association isn't a power of 2!\n");
			return 1;}
		if(assoc == 1){ assoc_type = DIRECT; } // treat it like a direct
		sets = cacheSize / (blockSize*assoc);
	}
	else {printf("Error, %s is not a valid cache associativity\n", argv[2]);
		return 1;}

	setIndexBits = powerOf2(sets); 
	
	// initializing cache with all empty blocks (zero)
	uint64_t cache [sets*assoc];
	uint64_t cache_prefetch[sets*assoc];	
	int i; for(i = 0; i<sets*assoc; i++){ cache[i] = 0; cache_prefetch[i] = 0;}
	
	// replacement policies
	if(strcmp(argv[3], "fifo")==0){rep_pol = FIFO;}
	else if(strcmp(argv[3], "lru")==0){rep_pol = LRU;}
	else {
		printf("Error, %s is not a valid replacement policy\n", argv[3]);
		return 0;
	}
	
	// making queue for replacement policies
	struct queue *q = queue_(sets, assoc, rep_pol);
	uint64_t container[sets*assoc];
	uint64_t container_prefetch[sets*assoc];
	for(i = 0; i<sets*assoc; i++){
		container[i] = 0;
		container_prefetch[i] = 0;
	}
	
	char *end_ptr; /* necessary for str to ll to work */
	int index;
	
	// scans input file for the three arguments in cache simulation
	while(fscanf(f,"%s %c %s\n", counter, &instruction, address) == 3){
		
		uint64_t decimalAddress = strtoll(address, &end_ptr, 16);
		index = getSetIndex(decimalAddress);
		
		/* DIRECT ASSOCIATION  -------------------------------------------------------------*/
		if(assoc_type == DIRECT){
			// cache miss N0 PRE-FETCH // 
			if(cache[index] == 0|| !compareTag(cache[index], decimalAddress)){ //empty cache block or conflict
				cache[index] = decimalAddress;
				CACHE_M++;
				MEM_R++;
				
				if(instruction == 'W')
					MEM_W++;
			}
			// cache hit N0 PRE-FETCH//
			else{
				CACHE_H++;
				if(instruction == 'W')
					MEM_W++;
			}
			// cache miss PRE-FETCH //
			if(cache_prefetch[index] == 0|| !compareTag(cache_prefetch[index], decimalAddress)){ //empty cache block or conflict
				cache_prefetch[index] = decimalAddress;
				
				if(!compareTag(cache_prefetch[(index+1)%sets],decimalAddress+blockSize)){ //prefetch
					cache_prefetch[(index+1)%sets] = decimalAddress+blockSize;
					MEM_R_P++;
				}
				MEM_R_P++;
				CACHE_M_P++;
				if(instruction == 'W')
					MEM_W_P++;
			}
			// cache hit PRE-FETCH//
			else{
				CACHE_H_P++;
				if(instruction == 'W')
					MEM_W_P++;
			}
		} 
		/* FULLY ASSOCIATED --------------------------------------------------------------- */
		else if(assoc_type == ASSOC){
			
			bool hit = false;
			
			// NO PRE-FETCH:
			for(i = 0; i<assoc; i++){
				
				if(compareTag(cache[i],decimalAddress)){ // -> find hit
					CACHE_H++; // update cache hits
					if(rep_pol == LRU){
						moveToFront(q, container, 0, decimalAddress); // move this address to top priority
					}
					if(instruction == 'W'){
						MEM_W++; // write-through
					}
					hit = true; // skip miss protocol
					break;
				}
			}
			// no hit, miss
			if(!hit){
				MEM_R++; // update memory reads
				CACHE_M++; // update cache misses
				
				// finds address to 
				uint64_t remove = bump(q, container, 0, decimalAddress); 
				
				// capacity miss, replace least relevant address
				if(remove != 0){
					for(i = 0; i<assoc; i++){
				
						if(compareTag(cache[i],remove)){ // --> find address queued for removal
							cache[i] = decimalAddress; // replaced least relevant address
							break;
					}}}
				
				// cold miss, find lowest in cache set to replace
				else{
					for(i = 0; i<assoc; i++){
						if(cache[i] == 0){
							cache[i] = decimalAddress;
							break;
					}}}
					
				if(instruction == 'W'){
					MEM_W++; // update memory writes
				}
			}
			hit = false; // good thing I didn't forget this
			
			
			// PRE-FETCH :
			for(i = 0; i<assoc; i++){
				
				if(compareTag(cache_prefetch[i],decimalAddress)){ // -> find hit
					CACHE_H_P++; // update cache hits
					if(rep_pol == LRU){
						moveToFront(q, container_prefetch, 0, decimalAddress); // move this address to top priority
					}
					if(instruction == 'W'){
						MEM_W_P++; // write-through
					}
					hit = true; // skip miss protocol
					break;
				}
			}
			// no hit, miss
			if(!hit){
				MEM_R_P++; // update memory reads
				CACHE_M_P++; // update cache misses
				
				bool skip_prefetch = false;
				
				// finds address to 
				uint64_t remove = bump(q, container_prefetch, 0, decimalAddress); 
				
				// capacity miss, replace least relevant address
				if(remove != 0){
					for(i = 0; i<assoc; i++){
				
						if(compareTag(cache_prefetch[i],remove)){ // --> find address queued for removal
							cache_prefetch[i] = decimalAddress; // replaced least relevant address
							break;
					}}}
				
				// cold miss, find lowest in cache set to replace
				else{
					for(i = 0; i<assoc; i++){
						
						if(cache_prefetch[i] == 0){
							cache_prefetch[i] = decimalAddress;
							break;
					}}}
				// prefetch next block
				for(i = 0; i<assoc; i++){ // search through cache for block after
					if(compareTag(cache_prefetch[i],decimalAddress+blockSize)){ 
						skip_prefetch = true;
						break;
					}}
				// if block after isn't in, add it
				if(!skip_prefetch){
					MEM_R_P++;
					
					remove = bump(q, container_prefetch, 0, decimalAddress+blockSize); 
					
					if(remove != 0){
						for(i = 0; i<assoc; i++){
				
							if(compareTag(cache_prefetch[i],remove)){ 
								cache_prefetch[i] = decimalAddress+blockSize; 
								break;
						}}}
					else{
						for(i = 0; i<assoc; i++){
						
							if(cache_prefetch[i] == 0){
								cache_prefetch[i] = decimalAddress+blockSize;
								break;
						}}}	
				}
				if(instruction == 'W'){ MEM_W_P++; }
			}
		}
		/* ASSOCIATED N --------------------------------------------------------------------*/
		else if(assoc_type == ASSOCN){
			
			bool hit = false;
			
			int i;
			for(i = 0; i<assoc; i++){
				if(compareTag(cache[(index*assoc)+i], decimalAddress)){ // cache hit
					CACHE_H++; // update cache hits
					if(rep_pol == LRU){
						moveToFront(q, container, index, decimalAddress); // move this address to top priority
					}
					if(instruction == 'W'){
						MEM_W++; // write-through
					}
					hit = true; // skip miss protocol
					break;
				}
			}
			// no hit, miss
			if(!hit){
				MEM_R++; // update memory reads
				CACHE_M++; // update cache misses
				uint64_t remove = bump(q, container, index, decimalAddress); // add address to queue
				if(remove != 0){
					for(i = 0; i<assoc; i++){
				
						if(compareTag(cache[(index*assoc)+i],remove)){ 
							cache[(index*assoc)+i] = decimalAddress; // replaced least relevant address
							break;
						}
					}
				}
				else{
					for(i = 0; i<assoc; i++){
						if(cache[(index*assoc)+i] == 0){
							cache[(index*assoc)+i] = decimalAddress; // replaced least relevant address
							break;
						}
					}
				}
				if(instruction == 'W'){
					MEM_W++; // update memory writes
				}
			}
			hit = false;
			
			// oh god, now for the precache assoc:n..
			for(i = 0; i<assoc; i++){
				if(compareTag(cache_prefetch[(index*assoc)+i], decimalAddress)){ // cache hit
					CACHE_H_P++; // update cache hits
					if(rep_pol == LRU){
						moveToFront(q, container_prefetch, index, decimalAddress); // move this address to top priority
					}
					if(instruction == 'W'){
						MEM_W_P++; // write-through
					}
					hit = true; // skip miss protocol
					break;
				}
			}
			// no hit, miss
			if(!hit){
				MEM_R_P++; // update memory reads
				CACHE_M_P++; // update cache misses
				uint64_t remove = bump(q, container_prefetch, index, decimalAddress); // add address to queue
				if(remove != 0){
					for(i = 0; i<assoc; i++){
				
						if(compareTag(cache_prefetch[(index*assoc)+i],remove)){ 
							cache_prefetch[(index*assoc)+i] = decimalAddress; // replaced least relevant address
							break;
						}
					}
				}
				else{
					for(i = 0; i<assoc; i++){
						if(cache_prefetch[(index*assoc)+i] == 0){
							cache_prefetch[(index*assoc)+i] = decimalAddress; // replaced least relevant address
							break;
						}
					}
				}
				if(instruction == 'W'){
					MEM_W_P++; // update memory writes
				}
			
			
				index = getSetIndex(decimalAddress+blockSize); // update index to next block
				bool skip_prefetch = false;
				for(i = 0; i<assoc; i++){
					if(compareTag(cache_prefetch[(index*assoc)+i], decimalAddress+blockSize)){ // cache hit
						skip_prefetch = true; // skip miss protocol
						break;
					}
				}
				if(!skip_prefetch){
					MEM_R_P++;
					
					uint64_t remove = bump(q, container_prefetch, index, decimalAddress+blockSize);
					
					if(remove!=0){
						for(i = 0; i<assoc; i++){
							if(compareTag(cache_prefetch[(index*assoc)+i],remove)){
								cache_prefetch[(index*assoc)+i] = decimalAddress+blockSize;
								break;
						}}}
					else{
						for(i = 0; i<assoc; i++){
							if(cache_prefetch[(index*assoc)+i] == 0){
								cache_prefetch[(index*assoc)+i] = decimalAddress+blockSize;
								break;
						}}}
				}
			}
		}
	}
	
	printf("no-prefetch\n");
	printf("Cache hits: %i\n", CACHE_H);
	printf("Cache misses: %i\n", CACHE_M);
	printf("Memory reads: %i\n", MEM_R);
	printf("Memory writes: %i\n", MEM_W);
	printf("with-prefetch\n");
	printf("Cache hits: %i\n", CACHE_H_P);
	printf("Cache misses: %i\n", CACHE_M_P);
	printf("Memory reads: %i\n", MEM_R_P);
	printf("Memory writes: %i\n", MEM_W_P);
	
	return 0;
}

// simple log2 calculator that returns -1 if input integer is not
// power of 2
int powerOf2(int x){
	int pow = 0;
	while (((x % 2) == 0) && x > 1){
		x /= 2;
		pow++;
	}
	if(x==1){return pow;}
	return -1;
}

// returns set index from memory address
int getSetIndex(long long mem_address){
	
	int ret; 
	if(setIndexBits == 0)
		return 1; // fully associative
	uint64_t mask = sets - 1;
	ret = (mem_address >> offsetBits)& mask;
	return ret;
	
}

// returns tag from memory address
uint64_t getTag(long long mem_address){
	return mem_address >> (offsetBits + setIndexBits);
}

// compares two memory addresses
bool compareTag(long long mem_arg_1, long long mem_arg_2){
	if(mem_arg_1 == 0)
		return false;
	
	uint64_t tag_1 = getTag(mem_arg_1);
	uint64_t tag_2 = getTag(mem_arg_2);
	
	if(tag_1 == tag_2)
		return true; 
	return false;
}

void printCache(uint64_t data[], int size){
	int i;
	for(i = 0; i<size; i++){
		printf("%" PRIu64" ", data[i]);
	}
	printf("\n");
}
