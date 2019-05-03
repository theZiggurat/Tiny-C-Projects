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
