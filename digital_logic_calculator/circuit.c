#include <stdbool.h>
#include <math.h>
#include <string.h>


#define SET_SIZE 64

enum varType{
	INPUT,
	OUTPUT,
	NEITHER,
	MULT
};

typedef struct var {
	enum varType v;
	char name[32];
	bool liveValue; 
} var;


typedef struct set {
	var container[SET_SIZE];
	int size;
} set;

enum directiveType{
	ROOT=0, NOT=1, AND=2, OR=3, NAND=4, NOR=5, XOR=6, DECODER=7, MULTIPLEXER=8, PASS=9
};

typedef struct directive {
	
	int args, outs, selectorCount;
	
	enum directiveType type; // type of operation
	struct directive *next;	 // links to next operation
	int inIndex[16];		 // input indexes of operation
	int outIndex[16];		 // output indexes of operation
	int selectors[64];		 // selectors if operation is MULTIPLEXER
	
} directive;

// DIRECTIVE CLASS //
//----------------------------------------------------------------------
// Circuit logic //
//----------------------------------------------------------------------
// constructor
directive *directive_(enum directiveType t, int args, int outs){
	
	directive *ret; 
	ret = malloc(sizeof(directive));
	
		ret->type = t;
		ret->inIndex[0] = 0;
		ret->outIndex[0] = 0;
		
		ret->args = args;
		ret->outs = outs;
		
}

// helper method
int power(int arg1, int arg2){
	
	int ret = 1, i;
	
	for(i = 0; i<arg2; i++){
		ret*=arg1;
	}
	
	return ret;
	
}

// helpder method
void printSpaces(int spaces){
	int i;
	
	for(i = 0; i<spaces; i++){
		printf(" ");
	}
}

// change var state
void cVState(set *s, int index, bool state){
	s->container[index].liveValue = state;
}

// get var state
bool gVState(set *s, int index){
	return s->container[index].liveValue;
}

void sortDirectives(directive *root, set *vars, int inputs, int outputs){
	
	int tempRoots = vars->size - 2 - inputs - outputs;
	
	if(tempRoots == 0){ // nothing to sort
		return;
	}
	
	int rootCounter = 0;
	int confirmedOutputs[tempRoots];
	
	directive *ptr;
	directive *back;
	directive *scout;
	
	back = root;
	ptr = root->next;
	int i, k;
	bool seen;
	
	while(ptr!=NULL){
		
		
		seen = false;
		
		// go through all directive inputs
		for(i = 0; i<ptr->args; i++){
			// input is a temp variable
			if(vars->container[ptr->inIndex[i]].v != INPUT){
				
				// check seen ouput temp variables 
				for(k = 0; k<tempRoots; k++){
					
					if(confirmedOutputs[k] == ptr->inIndex[i]){
						seen = true;
					}
					
				}
				
				// search later directives that have matching index as output
				if(!seen){
					
					scout = ptr;
					bool finished = false;
				
					while(scout->next!=NULL){
						if(finished){break;}
						for(k = 0; k<scout->next->outs; k++){
							// the directive after scout holds the output we need
							if(scout->next->outIndex[k] == ptr->inIndex[i]){
								// linked list switching
								
								directive *switchTemp;
								switchTemp = scout->next;
								
								if(switchTemp->next == NULL){
									scout->next = NULL;
								}
								else{
									scout->next = switchTemp->next;
								}
								
								back->next = switchTemp;
								switchTemp->next  = ptr;
								
								finished = true;
							}
							
						}
						if(!finished){
							scout = scout->next;
						}
					}
				}
			}
		}
		for(i = 0; i<ptr->outs; i++){
			
			seen = false;
			
			// output is a temp variable
			if(vars->container[ptr->outIndex[i]].v != OUTPUT){
				
				// add to confirmed outputs if not already there
				
				for(k = 0; k<rootCounter; k++){
					if(ptr->outIndex[i] == confirmedOutputs[k]){
						seen = true;
					}
				}
				
				if(!seen){
					confirmedOutputs[rootCounter] = ptr->outIndex[i];
					rootCounter++;
				}
				
			}
		}
		
		back = ptr;
		if(ptr->next==NULL){
			break;
		}
		ptr = ptr->next;
	}
	
	
}

// performs logic operation on directive
// depending on directivetype of the input directive
void crunch(struct directive *d, set *vars){
	
	if(d == NULL){
		return;
	}
	
	int q;
	int sum = 0;
	
	switch(d->type){
		
		case NOT: 
			cVState(vars, d->outIndex[0], !gVState(vars, d->inIndex[0])); 
			break;
			
		case AND:
			cVState(vars, d->outIndex[0], 
				gVState(vars, d->inIndex[0]) && gVState(vars, d->inIndex[1])); 
			break;
			
		case OR: 
			cVState(vars, d->outIndex[0], 
				gVState(vars, d->inIndex[0]) || gVState(vars, d->inIndex[1]));
			break;
			
		case NAND:
			cVState(vars, d->outIndex[0], 
				!(gVState(vars, d->inIndex[0]) && gVState(vars, d->inIndex[1])));
			break;
			
		case NOR: 
			cVState(vars, d->outIndex[0], 
				!(gVState(vars, d->inIndex[0]) || gVState(vars, d->inIndex[1])));
			break;
			
		case XOR:
			cVState(vars, d->outIndex[0], 
				(gVState(vars, d->inIndex[0]) || gVState(vars, d->inIndex[1]))
				&&!(gVState(vars, d->inIndex[0]) && gVState(vars, d->inIndex[1])));
			break;
			
		case DECODER: 
		
			sum = 0;
		
			for(q = 0; q<d->args; q++){
				//printf("%i %i\n", d->args-1-q, d->inIndex[d->args-1-q] ? 1:0);
				if(gVState(vars, d->inIndex[q])){
					sum += power(2, q);
				}
			}
			for(q = 0; q<d->outs; q++){
				
				if(q == sum){
					cVState(vars, d->outIndex[q], 1);
				}
				else{
					cVState(vars, d->outIndex[q], 0);
				}
			
			}
			
			break;
			
		case MULTIPLEXER:
		
			sum = 0;
		
			for(q = 0; q<d->args; q++){
				//printf("%i %i\n", d->args-1-q, d->inIndex[d->args-1-q] ? 1:0);
				if(gVState(vars, d->inIndex[q])){
					sum += power(2, q);
				}
			}
			cVState(vars, d->outIndex[0], gVState(vars, d->selectors[sum]));
			
			break;
			
		case PASS:
			cVState(vars, d->outIndex[0], gVState(vars, d->inIndex[0])); 
			break;
		
	}
	
	crunch(d->next, vars);
}

void printAllDirectives(directive *root){
	
	directive *scanner;
	scanner = root;
	scanner = scanner->next;
	
	while(scanner != NULL){
		
		printf("%i :: %i :: %i\n", 
				scanner->type, 
				scanner->args, 
				scanner->outs
		);
		
		int i;
		
		printf("{ ");
		for(i = 0; i<scanner->args; i++){
			printf("%i ", scanner->inIndex[i]);
		}
		printf(" }\n");
		printf("{ ");
		for(i = 0; i<scanner->outs; i++){
			printf("%i ", scanner->outIndex[i]);
		}
		printf(" }\n");
		
		if(scanner->selectorCount!=0){
			printf("{ ");
			for(i = 0; i<scanner->selectorCount; i++){
				printf("%i ", scanner->selectors[i]);
			}
			printf(" }\n");
		}
		
		scanner = scanner->next;
		
	}
}

// VAR CLASS //
//---------------------------------------------------------------------
// Encapsulates circuit data // 
//---------------------------------------------------------------------
// constructor
var *var_(enum varType v,char *name, bool initial_value){
	
	var *ret;
	ret = malloc(sizeof(var));
		ret->v = v;
		ret->liveValue = initial_value;
		
	strcpy(ret->name, name);
	
	return ret;
	
}

// destructor
void unvar_(var *v){
	free(v->name);
	free(&v->liveValue);
	free(v);
}

//
void changeValue(var *v, bool h){
	v->liveValue = h;
}

bool getValue(var *v){
	return v->liveValue;
}


// SET CLASS //
//---------------------------------------------------------------------
// Small set struct that holds 
//---------------------------------------------------------------------

// either returns index address of var already in set or 
// makes new var based on input string and varType and returns index
// of new var. returns -1 if set is full. 
int pushVar(set *s, char *str, const enum varType v){
	
	int i;
	var *temp;
	
	for(i = 0; i< s->size; i++){
		
		temp = &s->container[i];
		
		char *temp1 = temp->name;
		
		if(strcmp(temp1, str) == 0){
			return i;
		}
	}
	
	if(s->size >= SET_SIZE){
		// preventing segmentation fault
		return -1;
	}
	
	var *add;
	add = var_(v, str, false);
	
	s->container[s->size] = *add;
	s->size++;
	
	return s->size - 1;
}

// simple search that returns index var with name
// returns -1 if no var is found
int nameSearch(set *s, const char *str){
	
	int i;
	var *temp;
	char *tempstr;
	
	for(i = 0; i< s->size; i++){
		
		temp = &s->container[i];
		tempstr = temp->name;
		
		if(strcmp(tempstr, str) == 0){
			return i;
		}
	}
	return -1;
}



void printAllVars(const set *s){
	
	int i;
	
	for(i = 0; i<s->size; i++){
		
		printf("%s : %i : %i\n", 
			s->container[i].name, 
			i, 
			s->container[i].liveValue ? 1:0
		);
		
	}
}


