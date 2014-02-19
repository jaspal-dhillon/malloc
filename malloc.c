/* A simple allocation of the malloc routine */ 
#include<unistd.h> 
#include<stdio.h>
#include<pthread.h>
#define align4(x)	(((((x)-1)>>2)<<2)+4)
void *base=NULL;

pthread_mutex_t thread_lock;

typedef struct s_block* t_block;
struct s_block
{
	t_block next;
	size_t size;
	int free;
};

#define BLOCK_SIZE sizeof(struct s_block)

t_block extend_heap(t_block last, size_t s)
{
	t_block b = sbrk(0);
	if(sbrk(s+BLOCK_SIZE)==(void*)-1)
		return NULL;
	b->size=s;
	b->free=0;
	b->next=NULL;
	if(last)
		last->next=b;
	return b;
}

t_block find_block(t_block *last , size_t s)
{
	t_block b = base;
	while( b && !(b->free && b->size>=s)){
		*last=b;
		b=b->next;
	}
	return b;
}

void split_block(t_block b , size_t s)
{
	t_block new; 
	new = (t_block)((char*)b+BLOCK_SIZE+s);
	new->size = b->size - s - BLOCK_SIZE;
	new->free = 1;
	new->next = b->next;
	b->free = 0;
	b->next = new;
	return;
}

void* malloc(size_t size)
{
	//printf("yupieee, malloc got called finally, size_requested = %zu\n",size);
	size_t s = align4(size);
	t_block b,last;
	pthread_mutex_lock(&thread_lock);
	if(base==NULL){
		b = extend_heap(NULL,s);
		if(!b){
			pthread_mutex_unlock(&thread_lock);
			return NULL;
		}
		base=b;
	}
	else{
		b=find_block(&last,s);
		if(!b){
			b = extend_heap(last,s);
			if(!b){
				pthread_mutex_unlock(&thread_lock);
				return NULL;
			}
		}
		else{
			if(b->size-s>= sizeof(struct s_block)+4)
				split_block(b,s);
			b->free=0;
		}
	}
	pthread_mutex_unlock(&thread_lock);
	return (void*)((char*)b+BLOCK_SIZE);
}

t_block valid_addr(void *p, t_block *last)
{
	if(base){
		void *breakp = sbrk(0);
		if(!(p>base && p<breakp))
			return NULL;
		t_block b = base;
		*last=NULL;
		while(b && (void*)b<breakp && (void*)b<p){
			if((char*)b+BLOCK_SIZE==p)
				return b;
			*last=b;
			b=b->next;
		}
		return NULL;
	}
	return NULL;
}

void coalesce(t_block b)
{
	if(!(b && b->free==1 && b->next && b->next->free==1))
		return;
	b->size = b->size + BLOCK_SIZE + b->next->size;
	b->next = b->next->next;
	return;
}

void free(void *p)
{
	pthread_mutex_lock(&thread_lock);
	t_block last;
	t_block b = valid_addr(p,&last);
	if(!b){
		pthread_mutex_unlock(&thread_lock);
		return;
	}
	b->free = 1;
	if(b->next && b->next->free==1)
		coalesce(b);
	if(last && last->free==1)
		coalesce(last);
	pthread_mutex_unlock(&thread_lock);
	return;
}

void* realloc(void *p , size_t size)
{
	if(size==0){
		free(p);
		return p;
	}
	if(!p)
		return malloc(size);
	t_block last;
	t_block b = valid_addr(p,&last);
	if(!b)
		return p;
	size_t s = align4(size);
	if(b->size >= s){
		if(b->size > s+BLOCK_SIZE+4)
			split_block(b,s);
		return p;
	}
	void *new = malloc(s);
	if(!new)			 
		return NULL;
	size_t i,s4,*sp,*newp;
	s4=s/(sizeof(size_t));
	sp=(size_t*)(p);
	newp=(size_t*)(new);
	for(i=0;i<s4;i++)
		newp[i]=sp[i];
	free(p);
	return new;
}

int _helper_count_headers()
{
	if(!base)
		return 0;
	t_block b = base;
	int count = 0;
	while(b){
		count++;
		b = b->next;
	}
	return count;
}
