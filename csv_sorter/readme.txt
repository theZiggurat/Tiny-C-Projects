SimpleCSVSorter by Max Davatelis

Synopsis:
	cat [INPUT]|./sorter -c [SORT_PARAM]>[OUTPUT]
	
	[INPUT]: name of file used as CSV input (MUST be in csv file format)
	[SORT_PARAM]: column name to be sorted on
	[OUTPUT]: name out file to dump sorted CSV to
	
Errors:
	"Too many arguments": must use format above
	"Too little arguments": must use format above
	"Unknown argument qualifier": when -c is not used as argument
	"Column name not found": [SORT_PARAM] does not match any of the column names in input file
	"CSV input empty": parsing ended with empty data structure, input file must be empty

The design of the CSV sorter revolves around a 3D linked list data structure. Column nodes go up to down and contain row nodes. 
Row nodes go left to right and contain char nodes. Char nodes as a list represent a dynamically sized string. I made a small ASCII graphic 
to better explain it below:

	[root]
	  |
 	 [col]-[row]-[row]-[row]-...
	  | 	 |	   |     |
 	  |	   [char][char][char]
 	  | 	 |	   |     |
 	  |	   [char][char][char]
 	  | 	 |	   |   
	  |	   [char][char]
	  |			   |
 	  |	   		 [char]
 	  |
 	 [col]--...
	  |
	  |...
	  
Due to this design, it is possible to read the input file one character at a time, as all the program has to do is add more nodes 
on to a char node list. As a result, there were no assumptions made about the input file besides it being in standard CSV file format, so
this program can be ran on any CSV file (Note: some CSV files I've found online appear to not have a newline character at the end of every line.
This breaks the program completely and I didn't have enough time to make it work in every case). 
The file can be of any size with the data any type belonging to string or numeric fields. 

There are four main stages:

	* Parsing: Reads input file char by char. If it sees a newline, a new column node will be created and column pointer will move up.
			   If it sees a comma, a new row node will be created and row pointer will move up.
			   It if sees a regular character that's not a comma or newline, it will append it as a new character node to char pointer.
			   Data structure complete when reader reached EOF. 
			   
    * Rearranging: Read input argument. Increment through top row and find match between input argument and column name. Once found, move 
				   said column to the front such that at any column node, column->row will be the data to be sorted on. This is so the 
				   next stage will be able to access data to sort on easily. 
				   
    * Sorting: Mergesorts the list starting at the column node after the first row (which contains the column names). For data mergesort is 
			   comparing on, stringify_no_ws() will be used so all leading in trailing whitespaces won't matter for comparison.
    
    * Writng: Column by column: writes a row list
			  Row by row: converts row's char list to a string and writes it. If next row node is not null, writes a comma also. If next 
						  row node is null, writes a newline character.
						  
	  
	  
	  
