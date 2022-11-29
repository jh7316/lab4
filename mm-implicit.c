// This file gives you a starting point to implement malloc using implicit list
// Each chunk has a header (of type header_t) and does *not* include a footer
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "mm-common.h"
#include "mm-implicit.h"
#include "memlib.h"

// turn "debug" on while you are debugging correctness. 
// Turn it off when you want to measure performance
static bool debug = false; 

size_t hdr_size = sizeof(header_t);

void 
init_chunk(header_t *p, size_t csz, bool allocated)
{
	p->size = csz;
	p->allocated = allocated;
}

// Helper function next_chunk returns a pointer to the header of 
// next chunk after the current chunk h.
// It returns NULL if h is the last chunk on the heap.
// If h is NULL, next_chunk returns the first chunk if heap is non-empty, and NULL otherwise.
header_t *
next_chunk(header_t *h)
{
	//Your code here

	//return NULL if heap is empty
  	if(mem_heapsize()==0) return NULL;

	//then, return first chunk if h is NULL		
  	if(h==NULL)    return (header_t *)mem_heap_lo();
  	  

	//check if h is the last chunk of the heap
  	if((void *)((unsigned char *)h+(h->size))>=mem_heap_hi()) return NULL;
		
  	//otherwise, return the next chunk
	return (header_t *)((unsigned char *)h+(h->size));


}


/* 
 * mm_init initializes the malloc package.
 */
int mm_init(void)
{
	//double check that hdr_size should be 16-byte aligned
	assert(hdr_size == align(hdr_size));
	// start with an empty heap. 
	// no additional initialization code is necessary for implicit list.
	return 0;
}


// helper function first_fit traverses the entire heap chunk by chunk from the begining. 
// It returns the first free chunk encountered whose size is bigger or equal to "csz".  
// It returns NULL if no large enough free chunk is found.
// Please use helper function next_chunk when traversing the heap
header_t *
first_fit(size_t csz)
{
	//Your code here

	//return NULL if heap is empty
	if(mem_heapsize()==0) return NULL;

	
	header_t *curr= (header_t *)mem_heap_lo();

        while(curr!=NULL){
          bool alloc=curr->allocated;

          //if chunk is free and has size greater/equal to csz, return the chunk
          if((!alloc)) {
 	     if((curr->size)>=csz)
		return curr;
          }

           curr=next_chunk(curr);

        }
	
	//return NULL if none of the chunks meets the condition 
	return NULL;


}

// helper function split cuts the chunk into two chunks. The first chunk is of size "csz", 
// the second chunk contains the remaining bytes. 
// You must check that the size of the original chunk is big enough to enable such a cut.
void
split(header_t *original, size_t csz)
{
	//Your code here

	//if size of original chunk is not big enough, end function
	//if csz=original size, also end function
	if(csz>=(original->size)) return;

	size_t second=(original->size)-csz;	

	//reset the size of original to csz
	(original->size)=csz;

	
	//initialize new chunk to be the second part of the split chunk
	init_chunk((header_t *)((unsigned char *)original+(original->size)),second,(original->allocated));

}

// helper function ask_os_for_chunk invokes the mem_sbrk function to ask for a chunk of 
// memory (of size csz) from the "operating system". It initializes the new chunk 
// using helper function init_chunk and returns the initialized chunk.
header_t *
ask_os_for_chunk(size_t csz)
{
	//Your code here
	void *a=mem_sbrk(csz);
	init_chunk((header_t *)a, csz,false);
	return (header_t *)a;	

}

/* 
 * mm_malloc allocates a memory block of size bytes
 */
void *
mm_malloc(size_t size)
{

	//return NULL if size=0
	if(size==0) return NULL;

	//make requested payload size aligned
	size = align(size);
	//chunk size is aligned because both payload and header sizes
	//are aligned
	size_t csz = hdr_size +align(size);

	header_t *p = NULL;

	//Your code here 
	//to obtain a free chunk p to satisfy this request.
	//
	//The code logic should be:
	//Try to find a free chunk using helper function first_fit
	//    If found, split the chunk (using helper function split).
	//    If not found, ask OS for new memory using helper ask_os_for_chunk
	//Set the chunk's status to be allocated

	p=first_fit(csz);
	if(p!=NULL) split(p,csz);
	else p=ask_os_for_chunk(csz);
	p->allocated=true;



	//After finishing obtaining free chunk p, 
	//check heap correctness to catch bugs
	if (debug) {
		mm_checkheap(true);
	}
	return (void *)((unsigned char *)p+hdr_size);
}

// Helper function payload_to_header returns a pointer to the 
// chunk header given a pointer to the payload of the chunk 
header_t *
payload2header(void *p)
{
	//Your code here
	return (header_t *)((unsigned char *)p-hdr_size);


}

// Helper function coalesce merges free chunk h with subsequent 
// consecutive free chunks to become one large free chunk.
// You should use next_chunk when implementing this function
void
coalesce(header_t *h)
{
	//Your code here
	header_t *curr= next_chunk(h);


	//traverse the heap until it meets an allocated chunk 
        while(curr!=NULL&&(!curr->allocated)){
	  //add the current chunk size to h
	  (h->size)+=(curr->size);

           curr=next_chunk(curr);
        }	

}

/*
 * mm_free frees the previously allocated memory block
 */
void 
mm_free(void *p)
{
	// Your code here
	// 
	// The code logic should be:
	// Obtain pointer to current chunk using helper payload_to_header 
	// Set current chunk status to "free"
	// Call coalesce() to merge current chunk with subsequent free chunks
	  
	header_t *ptr=payload2header(p);
        ptr->allocated=false;
	coalesce(ptr);

	  
	// After freeing the chunk, check heap correctness to catch bugs
	if (debug) {
		mm_checkheap(true);
	}
}	

/*
 * mm_realloc changes the size of the memory block pointed to by ptr to size bytes.  
 * The contents will be unchanged in the range from the start of the region up to the minimum of   
 * the  old  and  new sizes.  If the new size is larger than the old size, the added memory will   
 * not be initialized.  If ptr is NULL, then the call is equivalent  to  malloc(size).
 * if size is equal to zero, and ptr is not NULL, then the call is equivalent to free(ptr).
 */
void *
mm_realloc(void *ptr, size_t size)
{
	// Your code here

	//if ptr==NULL, equivalent to malloc
	//if size==0, equivalent to free
	if(ptr==NULL) return mm_malloc(size);
        if(size==0){
          mm_free(ptr);
          return ptr;
        }

	
	header_t *p=payload2header(ptr);
	mm_free(ptr);

	//determine minimum of old/new sizes
	size_t min=p->size-hdr_size;
	if(min>size) min=size;

	//allocate a new memory block of size bytes
        void *newptr=mm_malloc(size);

	//copy the contents
	memcpy(newptr,ptr,min);
	
	
	// Check heap correctness after realloc to catch bugs
	if (debug) {
		mm_checkheap(true);
	}

	return newptr;
}


/*
 * mm_checkheap checks the integrity of the heap and returns a struct containing 
 * basic statistics about the heap. Please use helper function next_chunk when 
 * traversing the heap
 */
heap_info_t 
mm_checkheap(bool verbose) 
{
	heap_info_t info;
	// Your code here

	//set all statistics to 0
	(info.num_allocated_chunks)=0;
	(info.allocated_size)=0;
	(info.num_free_chunks)=0;
	(info.free_size)=0;


	// traverse the heap to fill in the fields of info

        header_t *curr= (header_t *)mem_heap_lo();

        while(curr!=NULL){
	  //boolean to determine if the chunk is free or not
	  bool alloc=curr->allocated;

	  //update statistics 
          if(alloc) {
    	       (info.num_allocated_chunks)++;
               (info.allocated_size)+=(curr->size);
	  }else{

               (info.num_free_chunks)++;
               (info.free_size)+=(curr->size);	
          }

 	   curr=next_chunk(curr); 
	   
	}

	// correctness of implicit heap amounts to the following assertion.
 	assert(mem_heapsize() == (info.allocated_size + info.free_size));
	return info;
}
