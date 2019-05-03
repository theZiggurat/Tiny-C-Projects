/*****
*
*	Define structures and function prototypes for your sorter
*
*	[root]
* 	  |
* 	 [col]-[row]-[row]-[row]
*	  | 	 |	   |     |
* 	  |	   [char][char][char]
* 	  | 	 |	   |     |
* 	  |	   [char][char][char]
* 	  | 	 |	   |   
* 	  |	   [char][char]
* 	  |			   |
* 	  |	   		 [char]
* 	  |
* 	 [col]- - - -
* 		
******/

#ifndef HEADER
#define HEADER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** TRUE AND FALSE */
#define TRUE 1
#define FALSE 0

/** for read() and write() functions **/
#define STDIN 0
#define STDOUT 1
#define STDERR 2

/** for compare() function in mergesort.c **/
#define STRING 0
#define FLOAT 1
#define INT 2

/** CHAR NODE STRUCT AND FUNCTION DECLARATIONS **/

/** linked list for dynamically sized string creation **/
typedef struct _cnode {
	char c;							// char data
	struct _cnode *next;			// next char node
} char_node;

char_node *cnode();					// char node constructor

void c_free(char_node *c);			// frees all char nodes attatched to root




/** ROW NODE STRUCT AND FUNCTION DECLARATIONS **/

typedef struct _rnode {
	int size;						// # of character nodes + '/0'
	struct _rnode *next;			// next row node
	char_node *chars;				// pointer to first char node
} row_node;

row_node *rownode();				// row node contructor

void r_free(row_node *root);     	// frees all row node & char nodes attatched to root

char *stringify(row_node *m);     	// converts row node to string

char *stringify_no_ws(row_node *m); // same as stringify without white spaces



/** COLUMN NODE STRUCT AND FUNCTION DECLARATIONS **/

typedef struct _colnode {
	row_node *row;					// pointer to first row node
	struct _colnode *next;			// pointer to next col node
} col_node;

col_node *colnode();				// column node constructor

void col_free(col_node *root);		// fress ALL nodes

/** WRITE FUNCTION DECLARATIONS **/

/** this is the master writing function. Takes in the root (which is a column node)
 * and writes everything attatched to it in csv file format **/
void write_CSV(col_node *root);

/** function that writes one row in csv file format **/
void write_list(row_node *list);

/** writes a column in list format. For debugging **/
void write_col_list(col_node *list);


/** MERGESORT FUNCTION DECLARATIONS (all belong in mergesort.c)**/

// master mergesort function (recursive)
col_node *mergesort(col_node *first);

// merges two column node lists
col_node *merge(col_node *ptr1, col_node *ptr2);

// splits a column node list in half
void split(col_node *in, col_node **a_out, col_node **b_out, int len);

// gets length of column node list
int listlength(col_node *root);

// compares two strings with the comparison depending on data type
// encoded in string
int compare(char *one, char *two);

#endif
