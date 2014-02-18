A very simple implementation of malloc(3) set of routines.

To compile: 
	./setup.sh 
To run:
	LD_LIBRARY_PATH=`pwd` ./a.out 

Some notes.

void* malloc(size_t size)
	Allocates memory using brk() using the first-fit algorithm.
	Splits blocks into smaller chunks if size of free block exceeds the requested size by at-least 4+sizeof(header) 

void free(void* p)
	Also coalesces free blocks together if possible. 
