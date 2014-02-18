#include<stdio.h>
#include<assert.h>
#include"header.h"
int main()
{
	int **d,i,j;
	d=(int**)malloc(10*sizeof(int*));
	assert(d);
	for(i=0;i<10;i++){
		d[i]=(int*)malloc(10*sizeof(int));
		assert(d[i]);
		for(j=0;j<10;j++)
			d[i][j]=i*j;
	}
	int *e = (int*)calloc(10,sizeof(int));
	assert(e!=NULL);
	printf("malloc(),calloc() successful.\n");
	e=realloc(e,20);
	assert(e!=NULL);
	printf("realloc() successful.\n");
	for(i=0;i<10;i++)
		free(d[i]);
	free(d);
	free(e);
	printf("free() successful.\n");
	assert(_helper_count_headers()==1);
	printf("Only 1 header block remains in allocator. coalesce() working as desired.\n");
	return 0;
}
