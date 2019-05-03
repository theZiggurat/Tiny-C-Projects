#include "simpleCSVsorter.h"


/** INPUT: 	-see simpleCSVsorter.h for data structure details
 *  	   	-this algorithm will assume data to be sorted is in the first 
 * 			-column of the CSV root
 **/
col_node *mergesort(col_node *first)
{
	
	if(!first)						// no data
		return NULL;
		
	int len = listlength(first);	// get length of lit
	
	if(len <= 1)					// single node, return it
		return first;
		
	col_node *ptr1;
	col_node *ptr2;
	
	/** the & operator on the two pointers are necessary as we want to 
	 *  assign an address to those two addresses that will persist after 
	 *  the split function returns **/
	split(first, &ptr1, &ptr2, len);

	ptr1 = mergesort(ptr1);			// recurse on sub-lists
	ptr2 = mergesort(ptr2);
	
	return merge(ptr1, ptr2);		// merge sorted sublists
}


col_node *merge(col_node *ptr1, col_node *ptr2)
{
	
	col_node *ret = NULL;			// can't have a 'root'
	col_node *merge_ptr = NULL;		// node since all nodes 
	while(ptr1 && ptr2)				// must have data
	{
		
		char *cmp1 = stringify_no_ws(ptr1->row);
		char *cmp2 = stringify_no_ws(ptr2->row);
		
		int k = compare(cmp1, cmp2);
		if(k)
		{
			if(ret == NULL)							// ret must be initialized with
			{										// first node in list 1
				ret = ptr1;							
				merge_ptr = ret;
				ptr1 = ptr1->next;
			} 
			
			else
			{
				merge_ptr->next = ptr1;				// attach node address to merge pointer
				ptr1 = ptr1->next;					// increment through first list
				merge_ptr = merge_ptr->next;
			}
		}
		
		else 
		{
			if(ret == NULL)							// ret must be initialized with 
			{										// first node in list 2
				ret = ptr2;
				merge_ptr = ret;
				ptr2 = ptr2->next;
			}
			
			else 
			{
				merge_ptr->next = ptr2;				// attach node address to merge pointer
				ptr2 = ptr2->next;					// increment through second list
				merge_ptr = merge_ptr->next;
			}
		}
	}
	
	if(ptr1)										// merge finished, but first list not empty
	{												// so append first list
		while(ptr1)
		{
			merge_ptr->next = ptr1;
			merge_ptr = merge_ptr->next;
			ptr1 = ptr1->next;
		}
	}
	else 											// merge finished, but second list not empty
	{												// so append second list
		while(ptr2)
		{
			merge_ptr->next = ptr2;
			merge_ptr = merge_ptr->next;
			ptr2 = ptr2->next;
		}
	}
	
	return ret;
}


/** master compare function that can take any string data, 
 * determine data type, and compare on that data type **/
 
int compare(char *one, char *two)
{
	if(one[0]==0)					// null data should be first in sorted list
		return 1;					// with a preference to original order
	else if(two[0]==0)
		return 0;
		
	int datatype;
	int i;
	
	char *copy;

	if(strlen(one)<strlen(two))		// decide data type from the longer data input
		copy = two;
	else
		copy = one;
	
	
	//determine data type
	for(i = 0; i<strlen(copy); i++)	// increment over which ever one is longer
	{
		char curr = *(copy+i);
		
		if(isalpha(curr))			// one character is all we need to see
		{							// to determine data type is string
			datatype = STRING;
			break;					// we now know data type is string
		} 
		
		else if(curr == '.')		// if we see period, we know number
		{							// is floating point
			datatype = FLOAT;
			break;					// we now know data type is float
		}
		
		else 						// might be int, might be float
		{							// we don't know unless we don't see '.'
			datatype = INT;			
		}							
	}
	
	// compare two values now that we know data type
	if(datatype == INT)
	{
		int a = atoi(one);			// convert string a to int
		int b = atoi(two);			// convert string b to int
		
		if (a<b)
			return 1;
		else
			return 0;
	} 
	
	else if(datatype == FLOAT)
	{
		double a = atof(one);		// convert string a to float
		double b = atof(two);		// convert string b to float
		
		if (a<b)
			return 1;
		else
			return 0;
	}
	
	else 
	{
		if(strcmp(one, two)>0)		// strcmp both strings
			return 0;
		else
			return 1;
	}
}


/** splits a column list into two (almost) equally sized column lists **/
void split(col_node *in, col_node **a_out, col_node **b_out, int len)
{
	int i;
	
	col_node *ptr = in, *prev;
	*a_out = in;
	
	for(i = 0; i<len/2; i++)
	{
		prev = ptr;
		ptr = ptr->next;
	}
	prev->next = NULL;
	*b_out = ptr;
}

/** gets total list length of column list **/
int listlength(col_node *first){
	
	int ret = 0;
	col_node *ptr = first;
	while(ptr!=NULL)
	{
		ptr = ptr->next;
		ret++;
	}
	return ret;
}
