#!/bin/sh
gcc -c -fPIC -Wall -Werror malloc.c -o malloc.o
gcc -shared -o libmalloc.so malloc.o 
gcc -L`pwd` -lmalloc -Wall test_malloc.c 
