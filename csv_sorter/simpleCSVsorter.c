#include "simpleCSVsorter.h"


int main(int argc, char **argv)
{
	
  if(argc > 3)
  {
	  printf("Too many arguments! Use -c *column*\n");
	  return -1;
  } 
  else if(argc < 3)
  {
	  printf("Too little arguments! Use -c *column*\n");
	  return -1;
  }
  
  int quotation_flag; 	// there can be commas in movie titles, so when 
						// program sees quotation, don't tokenize on comma
						
  int first_data_null;  // if a row starts with an empty field, this flag
						// will be toggled
						
  col_node *csv_root = colnode();	// this is the absolute root of all
									// the CSV data. csv_root->next
									// are the table categories
	
  char ch;							// buffer of size 1
  col_node *col_pointer = colnode();// connects (col -> col) and (col -> row)
  col_node *prevc;					// for snipping off last dangling node
  row_node *r_pointer = rownode();	// connects (row -> row)and (row -> char)
  char_node *c_pointer = cnode();		// connects (char -> char)
  
  csv_root->next = col_pointer;		// start point of CSV construction
  quotation_flag = FALSE;			// we don't want to ignore commas for now
  
  first_data_null = FALSE;
  
  
  
  // this while loops constructs the entire CSV file from the data file
  // to the 3D linked list data structure 
  
	  
  while(read(STDIN, &ch, 1)!=0){
	
	  if(ch == '"'){							// there will be a comma within a movie title
		  quotation_flag = 1 - quotation_flag; 	// toggle flag
	  }
	  
	  
	  /** ---------------------------------------------------------------------------*/
	  if(ch == '\n')							// new line, create new column node
	  { 			
		  col_pointer->next = colnode();		// move down column of CSV file
		  prevc = col_pointer;
		  col_pointer = col_pointer->next;	
	  } 
	  /** ---------------------------------------------------------------------------*/
	  
	  
	  
	  /** ---------------------------------------------------------------------------*/
	  else if (ch == ',' && !quotation_flag)	// new data entry, create new row node
	  { 		
		  
		  if(!col_pointer->row)					// column node was just allocated
		  {		
			  col_pointer->row = rownode();   	// so attatch row node to it
			  r_pointer = col_pointer->row; 	// and update row pointer
			  first_data_null = TRUE;
		  }	
		  else 
		  {
			  r_pointer->next = rownode();  	// if column node has row nodes,
			  r_pointer = r_pointer->next;		// attatch new row node to existing ones
		  }
		  
	  } 
	  /** ---------------------------------------------------------------------------*/
	  
	  
	  
	  /** ---------------------------------------------------------------------------*/
	  else 										// new char for entry, create new char node
	  {
		  if(!col_pointer->row)					// must be checked on first character read
		  {	
			  col_pointer->row = rownode();
			  r_pointer = col_pointer->row;	
		  }
		  
		  if(!r_pointer->chars)					// row node was just allocated
		  { 	
			  if(first_data_null)
			  {
				  r_pointer->next = rownode();
				  r_pointer = r_pointer->next;
				  first_data_null = FALSE;
			  }
			  r_pointer->chars = cnode();		// so attatch char node to it
			  c_pointer = r_pointer->chars; 	// and update char pointer
		  } 
		  
		  else 				
		  {			
			  c_pointer->next = cnode();		// if row node alreadt has char nodes,
			  c_pointer = c_pointer->next;		// attatch new char node to existing ones
		  }
		  
		  c_pointer->c = ch;					// insert char data
		  r_pointer->size++;					// and update size of string data
		  
	  }
	  /** ---------------------------------------------------------------------------*/
	  
  }
  
  if(!col_pointer->row)
		prevc->next = NULL;
  col_pointer = csv_root->next;	
 
  if(csv_root->next == NULL){					// CSV input must have been empty
	  write(STDERR, "CSV input empty!", 0);
	  return -1;
  }
  
  int found = FALSE, col_count = 0;
  char *col_name;
  
  
  if(strcmp(argv[1], "-c")==0)					// see if input args are correct
	col_name = argv[2];
  else 
	  printf("Unknown argument qualifier %s\n", argv[1]);
	  
  
  r_pointer = csv_root->next->row;				// get first row
  while(r_pointer){								// find column number
												// input argument belongs to
	  if(strcmp(stringify(r_pointer), col_name) == 0)
	  {
		  found = TRUE;
		  break;
	  }
	  
	  r_pointer = r_pointer->next;
	  col_count++;
  }
  
  
  if(!found)									// argument name not a column name
  {
	  printf("Column name %s not found\n", col_name);
	  return -1;
  }			
  
  
  row_node *prev;								// need prev to re-attatch rows
  
  int i;
  
  /** moves the column to be sorted to the first node in row list **/
  while(col_pointer)
  {
	  prev = NULL;
	  r_pointer = col_pointer->row;
	  
	  for(i = 0; i<col_count; i++){				// get to the correct column 
		  prev = r_pointer;						// within row list
		  r_pointer = r_pointer->next;	
	  }
	  
	  if(prev!=NULL){							// if prev is null, were moving
		  prev->next = r_pointer->next;			// first row to first
		  r_pointer->next = col_pointer->row;	 
		  col_pointer->row = r_pointer;
	  }
	  
	  col_pointer = col_pointer->next;
  }
  
  // mergesort assumes data to sort is in the first rownode of each column node
  // starts merge sort on the first data row onwards
  csv_root->next->next = mergesort(csv_root->next->next);
  
  fflush(stdout);								// don't know if it actually does anything
  write_CSV(csv_root);							// write the newly structured csv
  
  col_free(csv_root);							// frees all nodes

  return 0;										// job done
}


/** FUNCTION DEFINITIONS **/


/** MALLOC FUNCTIONS **/

/* char node constructor */
char_node *cnode(){
	char_node *ret = (char_node*)malloc(sizeof(char_node));
	return ret;
}

/* row node constructor */
row_node *rownode(){
	row_node *ret = (row_node*)malloc(sizeof(row_node));
	ret->size = 1;
	return ret;
}

/* column node constructor */
col_node *colnode(){
	col_node *ret = (col_node*)malloc(sizeof(col_node));
	ret->row = NULL;
	return ret;
}

/** prints entire data structure */
void write_CSV(col_node *root){
	
	
	if(!root)
	{
		return;
	}
	
	col_node *colptr = root->next;
	row_node *rowptr;
	
	while(colptr){
		
		write_list(colptr->row);
		
		write(STDOUT, "\n", 1);
		colptr = colptr->next;
	}
}

/** writes list of row nodes */
void write_list(row_node *list)
{
	row_node *rowptr = list;
	while(rowptr)
	{
		char *str = stringify(rowptr);
		write(STDOUT, str, strlen(str));
			
		if(rowptr->next!=NULL)
				write(STDOUT, ",", 1);
			
		rowptr = rowptr->next;
	}
}

/** writes list of column nodes, only useful in debugging */
void write_col_list(col_node *list)
{
	col_node *colptr = list;
	while(colptr)
	{
		
		char *str = stringify(colptr->row);
		write(STDOUT, str, strlen(str));
			
		if(colptr->next)
			write(STDOUT, ",", 1);
		//
		colptr = colptr->next;
	}
}

/** Converts row_node to string with null terminator **/
char *stringify(row_node *m){
	char *ret = (char*)malloc(sizeof(char)*m->size);
	char_node *chr = m->chars;
	
	if(m->chars == NULL){
		return "";
	}
	
	int i;
	for(i = 0; i< m->size-1; i++){
		
		if(chr == NULL){
			printf("No more char nodes for stringify!\n");
			exit(-1);
		}
		
		ret[i] = chr->c;
		chr = chr->next;
	}
	ret[m->size-1] = '\0';
	return ret;
}

/** converts row node to string with no trailing or leading white spaces or quotations*/
char *stringify_no_ws(row_node *m)
{
	char *ret = (char*)malloc(sizeof(char)*m->size);
	char_node *chr = m->chars;
	
	if(m->chars == NULL){
		return "";
	}
	
	int size = 0;
	int charfound = FALSE;
	int lastchar;
	
	int i;
	for(i = 0; i< m->size-1; i++){
		
		if((chr->c==' ' && !charfound)||chr->c == '"')
		{
			chr = chr->next;
			continue;
		} 
		else 
			lastchar++;
	
	
		if(chr == NULL){
			printf("No more char nodes for stringify!\n");
			exit(-1);
		}
		
		ret[size++] = chr->c;
		chr = chr->next;
		charfound = TRUE;
	}
	
	for(i = size; i<=lastchar; i++)
		ret[i] = 0;
	
	
	return ret;
}

/** CLEANUP FUNCTIONS **/

/** frees everything */
void col_free(col_node *root){
	col_node *ptr = root->next;
	col_node *prev = root;
	
	
	while(ptr!=NULL){
		if(ptr->row!=NULL){
			r_free(ptr->row);
		}
		free(prev);
		prev = ptr;
		ptr = ptr->next;
	}
	
	if(prev!=NULL){
		free(prev);
	}
}

/** frees row nodes and attatched char nodes */
void r_free(row_node *root)
{
	row_node *ptr = root->next;
	row_node *prev = root;
	
	while(ptr!=NULL){
		if(ptr->chars!=NULL){
			c_free(ptr->chars);
		}
		free(prev);
		prev = ptr;
		ptr = ptr->next;
	}
	
	if(prev != NULL){ 
		free(prev);
	}
}

/** frees char nodes */
void c_free(char_node *c){
	char_node *ptr = c->next;
	char_node *prev = c;
	
	while(ptr!=NULL){
		free(prev);
		prev = ptr;
		ptr = ptr->next;
	}
	
	if(prev!=NULL){
		free(prev);
	}
}

