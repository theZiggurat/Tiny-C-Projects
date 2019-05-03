Cachesim by Max Davatelis

Synopsis:
	./cachesim [CACHE_SIZE] [ASSOCIATIVITY] [REPLACEMENT_POLICY] [BLOCK_SIZE] [TRACE_FILE]
	
	[CACHE_SIZE]: The cache size, in bytes. This will be a power of two
	[ASSOCIATIVITY]: The associativity, which will be “direct” for a direct-mapped cache, “assoc:n” for an n-way
associative cache, or “assoc” for a fully associative cache
	[REPLACEMENT_POLICY]: The replacement policy, which will be “fifo” or “lru”
	[BLOCK_SIZE]: The block size, in bytes. This will be a power of two.
	[TRACE_FILE]: Relative directory of trace file
