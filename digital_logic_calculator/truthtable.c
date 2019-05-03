#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "circuit.c"

bool fileInput = true;
int inputs = 0, outputs = 0;
bool header = false;

int main(int argc, char **argv){
	
	
	char *input = argv[1];
	
	if(input == NULL)
		fileInput = false;
	else if(strcmp(input, "-h")==0)
	{
			header = true;
			if(argc >= 3)
				input = argv[2];
			else 
				fileInput = false;
	}
	
	FILE *f;

	directive *root;
	root = directive_(ROOT, 0, 0);
	
	directive *ptr;
	ptr = root;
	
	// allocating set with one and zero for multiplexer
	set vars;
	vars.size = 0;
	pushVar(&vars, "__mult__zero__", MULT);
	pushVar(&vars, "__mult__one__", MULT);
	
	vars.container[1].liveValue = true;
	
	f = fopen(input, "r");
	if(!f)
	{
		printf("File does not exist!\n");
		exit(EXIT_FAILURE);
	}
	
	// CONSTRUCTING DIRECTIVE LINKED LIST 
	char str[10];
	char args[100];
	
	while(!feof(f)){
		
		fscanf(f, "%s ", &str);
		
		
		char tmpstr[15];
		int i, x;
		
		// INPUT		
		if (strcmp(str, "INPUT") == 0) {
			
			fscanf(f, "%i ", &i);
			var *store;
			
			
			for(x = 0; x<i-1; x++){
				fscanf(f, "%s ", &tmpstr);
				pushVar(&vars, tmpstr, INPUT);
			}
			
			fscanf(f, "%s\n", &tmpstr);
			pushVar(&vars, tmpstr, INPUT);
			
			inputs = i;

		} 
		// OUTPUT			
		else if(strcmp(str, "OUTPUT") == 0){
			
			fscanf(f, "%i ", &i);
			var *store;
			
			
			for(x = 0; x<i-1; x++){
				fscanf(f, "%s ", &tmpstr);
				pushVar(&vars, tmpstr, OUTPUT);
			}
			
			fscanf(f, "%s\n", &tmpstr);
			pushVar(&vars, tmpstr, OUTPUT);
			
			outputs = i;
			
		}
		// NOT		
		else if(strcmp(str, "NOT") == 0){
			
			directive *tmp_not;
			tmp_not = directive_(NOT, 1, 1);
			
			fscanf(f, "%s ", &tmpstr);
			tmp_not->inIndex[0] = pushVar(&vars, tmpstr, NEITHER);
			
			fscanf(f, "%s ", &tmpstr);
			tmp_not->outIndex[0] = pushVar(&vars, tmpstr, NEITHER);

			ptr->next = tmp_not;
			ptr = ptr->next;
			
		}
		// AND		
		else if(strcmp(str, "AND") == 0){
			
			directive *tmp_and;
			tmp_and = directive_(AND, 2, 1);
			
			fscanf(f, "%s ", &tmpstr);
			tmp_and->inIndex[0] = pushVar(&vars, tmpstr, NEITHER);
			
			fscanf(f, "%s ", &tmpstr);
			tmp_and->inIndex[1] = pushVar(&vars, tmpstr, NEITHER);
			
			fscanf(f, "%s ", &tmpstr);
			if(strcmp(tmpstr, ":") == 0){
				fscanf(f, "%s \n", &tmpstr);
			}
			tmp_and->outIndex[0] = pushVar(&vars, tmpstr, NEITHER);

			ptr->next = tmp_and;
			ptr = ptr->next;
			
		}
		// OR		
		else if(strcmp(str, "OR") == 0){
			
			directive *tmp_or;
			tmp_or = directive_(OR, 2, 1);
			
			fscanf(f, "%s ", &tmpstr);
			tmp_or->inIndex[0] = pushVar(&vars, tmpstr, NEITHER);
			
			fscanf(f, "%s ", &tmpstr);
			tmp_or->inIndex[1] = pushVar(&vars, tmpstr, NEITHER);
			
			fscanf(f, "%s ", &tmpstr);
			if(strcmp(tmpstr, ":") == 0){
				fscanf(f, "%s \n", &tmpstr);
			}
			tmp_or->outIndex[0] = pushVar(&vars, tmpstr, NEITHER);

			ptr->next = tmp_or;
			ptr = ptr->next;
			
		}
		// NAND			
		else if(strcmp(str, "NAND") == 0){
			
			directive *tmp_nand;
			tmp_nand = directive_(NAND, 2, 1);
			
			fscanf(f, "%s ", &tmpstr);
			tmp_nand->inIndex[0] = pushVar(&vars, tmpstr, NEITHER);
			
			fscanf(f, "%s ", &tmpstr);
			tmp_nand->inIndex[1] = pushVar(&vars, tmpstr, NEITHER);
			
			fscanf(f, "%s ", &tmpstr);
			if(strcmp(tmpstr, ":") == 0){
				fscanf(f, "%s ", &tmpstr);
			}
			tmp_nand->outIndex[0] = pushVar(&vars, tmpstr, NEITHER);

			ptr->next = tmp_nand;
			ptr = ptr->next;
			
		}
		// NOR		
		else if(strcmp(str, "NOR") == 0){
			
			directive *tmp_nor;
			tmp_nor = directive_(NOR, 2, 1);
			
			fscanf(f, "%s ", &tmpstr);
			tmp_nor->inIndex[0] = pushVar(&vars, tmpstr, NEITHER);
			
			fscanf(f, "%s ", &tmpstr);
			tmp_nor->inIndex[1] = pushVar(&vars, tmpstr, NEITHER);
			
			fscanf(f, "%s ", &tmpstr);
			if(strcmp(tmpstr, ":") == 0){
				fscanf(f, "%s ", &tmpstr);
			}
			tmp_nor->outIndex[0] = pushVar(&vars, tmpstr, NEITHER);

			ptr->next = tmp_nor;
			ptr = ptr->next;
			
		}
		// XOR			
		else if(strcmp(str, "XOR") == 0){
			
			directive *tmp_xor;
			tmp_xor = directive_(XOR, 2, 1);
			
			fscanf(f, "%s ", &tmpstr);
			tmp_xor->inIndex[0] = pushVar(&vars, tmpstr, NEITHER);
			
			fscanf(f, "%s ", &tmpstr);
			tmp_xor->inIndex[1] = pushVar(&vars, tmpstr, NEITHER);
			
			fscanf(f, "%s ", &tmpstr);
			if(strcmp(tmpstr, ":") == 0){
				fscanf(f, "%s ", &tmpstr);
			}
			tmp_xor->outIndex[0] = pushVar(&vars, tmpstr, NEITHER);

			ptr->next = tmp_xor;
			ptr = ptr->next;
			
		}
		// DECODER			
		else if(strcmp(str, "DECODER") == 0){
			
			int varCount;
			int x;
			
			fscanf(f, "%i ", &varCount);
			fscanf(f, "%s", &tmpstr); // delim ':'
			
			i = power(2, varCount);
			
			directive *tmp_decoder;
			tmp_decoder = directive_(DECODER, varCount, i);
			
			for(x = 0; x<varCount; x++){
				
				if(strcmp(tmpstr, ":")==0){
					fscanf(f, "%s", &tmpstr);
				}
				
				tmp_decoder->inIndex[x] = pushVar(&vars, tmpstr, NEITHER);
				
				fscanf(f, "%s", &tmpstr);
			}
			
			fscanf(f, "%s", &tmpstr);
			
			for(x = 0; x<i; x++){
				
				if(strcmp(tmpstr, ":")==0){
					fscanf(f, "%s", &tmpstr);
				}
				
				tmp_decoder->outIndex[x] = pushVar(&vars, tmpstr, NEITHER);
				
				fscanf(f, "%s", &tmpstr);
			}
			
			fscanf(f, "\n", &tmpstr);
			
			ptr->next = tmp_decoder;
			ptr = ptr->next;
			
		}
		// MULTIPLEXER		
		else if(strcmp(str, "MULTIPLEXER") == 0){
			
			int varCount;
			
			fscanf(f, "%i ", &varCount);
			fscanf(f, "%s", &tmpstr); // delim ':'
			
			i = power(2, varCount);
			
			directive *tmp_mult;
			tmp_mult = directive_(MULTIPLEXER, varCount, 1);
				tmp_mult->selectorCount = i;
			
			for(x = 0; x<i; x++){
				
				if(strcmp(tmpstr, ":")==0){
					fscanf(f, "%s", &tmpstr);
				}
				
				if(strcmp(tmpstr, "0")==0){
					tmp_mult->selectors[x] = 0; // address of 0 const
				} 
				else if(strcmp(tmpstr, "1")==0){
					tmp_mult->selectors[x] = 1; // address of 1 const
				} 
				else {
					// non-constant var for selector 
					tmp_mult->selectors[x] = pushVar(&vars, tmpstr, NEITHER);
				}
				
				fscanf(f, "%s", &tmpstr);
			}
			
			fscanf(f, "%s", &tmpstr); // delim ':'
			
			for(x = 0; x<varCount; x++){
				
				if(strcmp(tmpstr, ":")==0){
					fscanf(f, "%s", &tmpstr);
				}
				
				tmp_mult->inIndex[x] = pushVar(&vars, tmpstr, NEITHER);
				
				fscanf(f, "%s", &tmpstr);
			}
			
			fscanf(f, "%s", &tmpstr); // delim ':'
			
			if(strcmp(tmpstr, ":")==0){
				fscanf(f, "%s", &tmpstr);
			}
			
			tmp_mult->outIndex[0] = pushVar(&vars, tmpstr, NEITHER);
			
			fscanf(f, "\n", &tmpstr);
			
			ptr->next = tmp_mult;
			ptr = ptr->next;
			
		}
		// PASS			
		else if(strcmp(str, "PASS") == 0){
			
			directive *tmp_pass;
			tmp_pass = directive_(PASS, 1, 1);
			
			fscanf(f, "%s ", &tmpstr);
			tmp_pass->inIndex[0] = pushVar(&vars, tmpstr, NEITHER);
			
			fscanf(f, "%s ", &tmpstr);
			tmp_pass->outIndex[0] = pushVar(&vars, tmpstr, NEITHER);

			ptr->next = tmp_pass;
			ptr = ptr->next;
			
		}
		
	}
	
	int u, w, r;
	// to hold spacing of table
	int spacing[inputs + outputs];
	
	// HEADER	
	if(header){
		
		for(u = 0; u < inputs + outputs; u++){
			spacing[u] = strlen(vars.container[2+u].name);
		}
		
		for(u = 0; u < inputs; u++){
			printf("%s ", vars.container[2+u].name);
		}	printf("| ");
		for(u = 0; u < outputs; u++){
			printf("%s ", vars.container[2+inputs+u].name);
		}	printf("\n");
		
	}	
	
	// PRINT & LOGIC SIMULATION // 
	for(u = 0; u< power(2, inputs); u++){
		
		// loads the 2 bit version of counter u into 
		// the liveValue of each var of the input set
		
		for(r = 0; r<inputs ; r++){
			if(header){
				printSpaces(spacing[r]-1);
			}
			if(u/power(2,inputs-r-1)%2 == 0){
				vars.container[2+r].liveValue = false;
				printf("0");
			} 
			else {
				vars.container[2+r].liveValue = true;
				printf("1");
			}
			printf(" ");
		}
		printf("| ");
		
		// updates output liveValues
		crunch(root, &vars);
		
		// copies output to log
		for(r = 0; r<outputs; r++){
			
			if(header)
				printSpaces(spacing[r+inputs]-1);
			
			if(!vars.container[2+inputs+r].liveValue)
				printf("0");
			else 
				printf("1");
			
			printf(" ");
		}
		
		printf("\n");
		
	}
	
	return EXIT_SUCCESS;
}
