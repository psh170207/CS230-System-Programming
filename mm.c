/*
 * mm.c - For our dynamic memory allocator, use seggregated free list.
 * 
 * In this program, there is only two global variable.
 * One global is 'heap_listp' which is pointer of first byte of heap block lists.
 * Another global is 'list' which is pointer of base pointer of seggregated free list.
 * This 'list' is regarded as base pointer of array of doubly linked lists(free lists). For example, 
 * list[0]( = *(char **)list ) is first free list and list[1]( = *((char **)list + 1*sizeof(pointer)) ) is second
 * free list and so on. And each list[i] has head of (i+1)-th free list.
 *
 * In this seggregated free list approach, each free list has class of size.
 * Since the minimum essential free block size is 24Bytes( = header(4B) + footer(4B) + next_ptr(8B) + prev_ptr(8B) ),
 * first free list has size class (only 24B). And other i-th free list has size class (2^(i+2)+1 Bytes to 2^(i+3) Bytes).
 * For example, 9th free list has size class (2049Bytes to 4096Bytes). We have 17 size classes.
 *
 * Every newly generated free block inserted to head of corresponing size class free list. For example, newly freed block has size
 * 4080 Bytes, this block will be inserted to head of list[8].
 * Also, every finding free block for allocation, searching the block from corresponding size class free list. First-fit policy is choosen.
 * If there's no available free block in that free list, searching the block from the next size class free list(bigger size class).
 * At the end, there's no fit free block, extend heap size and allocate to extended heap area.
 * 
 * In coalescing and placing, it may need to remove free block from free list and add newly generated free block to free list.
 * Removing and adding to free list follows basic doubly linked list's rule. But in this approach, LIFO policy is choosen.
 * Thus, adding a new element to free list is add to head of doubly linked list. 
 * In removing, we have to consider 4 cases. More detail, in source code.
 * 
 * There's some macros for manipulating the free lists. More detail, in source code.
 * For other detailed description of functions, please read header comment of each functions.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "cs20150326",
    /* First member's full name */
    "Park Si Hwan",
    /* First member's email address */
    "psh150204@kaist.ac.kr",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE 4 /*word and header/footer size (bytes)*/
#define DSIZE 8 /*Double word size (bytes)*/
#define CHUNKSIZE (1<<12) /*Extend heap by this amount (bytes)*/

#define MAX(x,y) ((x) > (y) ? (x) : (y))

/*Pack a size and allocated bit into a word*/
#define PACK(size, alloc) ((size)|(alloc))

/*Read and write a word at address p*/
#define GET(p) (*(unsigned int *)(p))
#define PUT(p,val) (*(unsigned int *)(p) = (val))

/*Read the size and allocate fields from address p*/
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/*Given block ptr bp, compute address of its header and footer*/
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/*Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((HDRP(bp) - WSIZE)))

/*FOR EXPLICIT FREE LIST*/
/*Given block ptr bp, compute address of next and prev block */
#define NPTR(bp) (*(char **)(bp)) // In Doubly Linked List, get the NEXT Linked block's POINTER
#define PPTR(bp) (*(char **)((char *)bp + 2*WSIZE)) // as same as NPTR, but get the PREV Linked block's POITNER : P(REV) P(OIN)T(E)R, pointer is 8 bytes

/*Write a pointer at address p*/
#define PUT_NPTR(bp,np) (NPTR(bp) = (np)) // Link NEXT POINTER of np to block bp
#define PUT_PPTR(bp,pp) (PPTR(bp) = (pp)) // Link PREV POINTER of pp to block bp

/* FOR SEGGREGATED FREE LIST */
#define CNUM 17 /*Number of size classes*/
#define CPTR(bp) *(char **)(bp)// GET head of doubly linked list with base pointer bp : C(LASS)P(OIN)T(E)R
#define PUT_CPTR(bp,hp) (*(char **)(bp) = (hp)) // PUT hp(head pointer) to doubly linked list with base pointer bp

/*

Structure of initial heap

+				 +
+----------------+
+				 +
+----------------+
+		0      |1+ <- epilogue block
+----------------+
+     DSIZE    |1+ <- prologue block footer : heap_listp
+----------------+
+     DSIZE    |1+ <- prologue block header
+----------------+
+		0		 + <- padding
+----------------+
+	  list16	 + <- list + 16*DSIZE : base pointer of list16
+----------------+ 
+				 +
+  more blocks   + <- list + idx * DSIZE : base pointer of list idx
+				 +
+----------------+ 
+	  list2  	 + <- list + 2*DSIZE : base pointer of list2
+----------------+ 
+	  list1 	 + <- list + 1*DSIZE : base pointer of list1
+----------------+ 
+	  list0		 + <- list : base pointer of list0
+----------------+ 
*/

/*

Structure of free block

+				 + <- block pointer of next block
+----------------+
+				 + <- header of next block
+----------------+
+	   size    |0+ <- footer of free block
+----------------+
+				 + 
+----------------+
+				 +
+	more blocks	 +
+				 +
+----------------+
+	 PREV PTR	 + <- contain PREV FREE LISTED BLOCK POINTER
+----------------+
+	 NEXT PTR	 + <- block pointer of free block and contain NEXT FREE LISTED BLOCK POINTER
+----------------+
+	   size    |0+ <- header of free block
+----------------+
+				 + <- footer of prev block
+----------------+
+				 + 
+----------------+
+				 +

*/

/*

Structure of allocated block

+				 + <- block pointer of next block
+----------------+
+				 + <- header of next block
+----------------+
+	   size    |1+ <- footer of this block
+----------------+
+	 contents	 + 
+----------------+
+				 +
+	more blocks	 +
+				 +
+----------------+
+	 contents	 + 
+----------------+
+	 contents	 + <- block pointer of this block 
+----------------+
+	   size    |1+ <- header of this block
+----------------+
+				 + <- footer of prev block
+----------------+
+				 + 
+----------------+
+				 +

*/

static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
static void add_list(void *bp);
static void remove_list(void *bp);

static int mm_check(void);
static int isListed(void *bp);
static int isValid(void *bp);

static void *heap_listp;
static void *list;

/*
 * BPTR - return the base pointer(address) of given size class(represented by index) list
 */
static void *BPTR(int idx)
{
	return ((char *)list+idx*DSIZE);
}

/*
 * class_idx - return the size class(index) from asize
 */
static int class_idx(unsigned int asize)
{
	int idx = -1;

	if(asize==24) idx = 0;
	else if(asize<=32) idx = 1;
	else if(asize<=64) idx = 2;
	else if(asize<=128) idx = 3;
	else if(asize<=256) idx = 4;
	else if(asize<=512) idx = 5;
	else if(asize<=1024) idx = 6;
	else if(asize<=2048) idx = 7;
	else if(asize<=4096) idx = 8;
	else if(asize<=8192) idx = 9;
	else if(asize<=16384) idx = 10;
	else if(asize<=32768) idx = 11;
	else if(asize<=65536) idx = 12;
	else if(asize<=131072) idx = 13;
	else if(asize<=262144) idx = 14;
	else if(asize<=524288) idx = 15;
	else idx = 16;
	
	return idx;
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
	void *ptr;
	void *base;
	int i;

	/* create the initial empty heap */
	if ((list = mem_sbrk((CNUM+2)*DSIZE)) == (void *)-1)
		return -1;
	
	/* set each size class list's head to NULL (create the initial empty size class lists) */
	for(i=0;i<CNUM;i++){
		base = BPTR(i);
		PUT_CPTR(base,NULL);
	}
	
	heap_listp = list + CNUM*DSIZE;

	PUT(heap_listp, 0); /*Alignment padding*/
	PUT(heap_listp + (1*WSIZE), PACK(DSIZE,1));/*Prologue header*/
	PUT(heap_listp + (2*WSIZE), PACK(DSIZE,1));/*Prologue footer*/
	PUT(heap_listp + (3*WSIZE), PACK(0,1)); /*Epliogue header*/
	
	heap_listp += (2*WSIZE);
	
	/*Extend the empty heap with a free block of CHUNKSIZE bytes*/
	if((ptr = extend_heap(CHUNKSIZE/WSIZE)) == NULL)
		return -1;
	
	base = BPTR(8); // size class 8 contains free block with size 4096
	PUT_CPTR(base,ptr); // set the initial free block
	PUT_NPTR(CPTR(base),NULL); // set the initial free block's NEXT to NULL
	PUT_PPTR(CPTR(base),NULL); // set the initial free block's PREV to NULL

	return 0;
}

/*
 * extend_heap - If there's no available free blocks, extend the heap size.
 *		And newly generated free block coalesce with previous blocks when previous block is already freed.
 */

static void *extend_heap(size_t words)
{
	char *bp;
	size_t size;
	
	/* Allocate an even number of words to maintain alignment */
	size = (words % 2) ? (words+1) *WSIZE : words * WSIZE;
	if((long)(bp = mem_sbrk(size)) == -1)
		return NULL;

	/* Initialize free block header/footer and the epilogue header */
	PUT(HDRP(bp), PACK(size,0)); /* Free block header */
	PUT(FTRP(bp), PACK(size,0)); /* Free block footer */
	PUT(FTRP(bp)+WSIZE, PACK(0,1)); /* New epilogue header */
	
	/* Coalesce if the previous block was free */
	return coalesce(bp);
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
	size_t asize; /*Adjusted block size*/
	size_t extendsize; /*Amount to extend heap if no fit*/
	char *bp;
	
	/*Ignore spurious request*/
	if(size == 0)
		return NULL;

	/*Adjust block size to include overhead and alignment reqs*/
	if(size <= DSIZE)
		asize = 2*DSIZE;
	else
		asize = DSIZE * ((size + DSIZE + (DSIZE-1)) / DSIZE);

	/*Search the free list for a fit*/
	if((bp = find_fit(asize)) != NULL){
		place(bp, asize);
		return bp;
	}

	/*No fit found, Get more memory and place the block*/
	extendsize = MAX(asize,CHUNKSIZE);
	if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
		return NULL;
	
	add_list(bp); // add bp to free list : bp is first byte of extended heap, and entire block is free.

	place(bp,asize);
	
	return bp;
}

/*
 * find_fit - find free block for allocation from size class free lists.
 */

static void *find_fit(size_t asize)
{
	/*First-fit Search*/
	
	void *bp;
	void *base;
	int i = class_idx(asize); // i is index of size class which determined by asize.
	
	/* For searching free blocks, if there's no fit free block, searching free block in larger size class list */
	while(i<CNUM){
		base = BPTR(i);
		for(bp = CPTR(base); bp!=NULL; bp = NPTR(bp)){
			if(GET_SIZE(HDRP(bp))>=asize) return bp;
		}
		i++;
	}

	/* No fit free block found, return NULL */
	return NULL;
}

/*
 * place - manage the newly allocating block(set header, footer, remove to free list,..) 
 *		add newly generated free block(splitted free block) to corresponding size class list.
 */
static void place(void *bp, size_t asize)
{
	size_t csize = GET_SIZE(HDRP(bp));
	
	remove_list(bp);

	/* We need at least 24 bytes(3 double words) for free block to set header(4bytes)/footer(4bytes)/next block ptr(8bytes)/prev block ptr(8bytes) */
	if((csize - asize) >= 3*DSIZE){
	/* CASE1 : if rest of free block is big enough, allocate given asize and split rest free block */

		PUT(HDRP(bp), PACK(asize,1));
		PUT(FTRP(bp), PACK(asize,1));

		bp = NEXT_BLKP(bp); // move bp to bp+asize
		PUT(HDRP(bp), PACK(csize-asize, 0)); //new free block's header
		PUT(FTRP(bp), PACK(csize-asize, 0)); //new free block's footer

		add_list(coalesce(bp)); //coalescing the newly generated free block(splitted free block) and add to free list.
	}
	else{
	/* CASE2 : if rest of free block is not enough, just allocate entire free block */
		PUT(HDRP(bp), PACK(csize,1));
		PUT(FTRP(bp), PACK(csize,1));
	}
}

/*
 * remove_list - remove given block(bp) from size class list (size class list determined by bp's size)
 */

static void remove_list(void *bp)
{	
	size_t size = GET_SIZE(HDRP(bp));
	int i = class_idx(size);
	void *base = BPTR(i);

	if(NPTR(bp) == NULL && PPTR(bp) == NULL){
	/* CASE1 : There is only one elelment in its free list */	

		/* Initialize to NULL (remove allocated block from list) */	
		PUT_CPTR(base,NULL);
	}
	else if(NPTR(bp) != NULL && PPTR(bp) == NULL){
	/* CASE2 : The allocated block is first element of current list */
		
		PUT_PPTR(NPTR(bp),NULL); //set the next block's prev to NULL
		
		/* set the new head of current list */
		PUT_CPTR(base,NPTR(bp));	

		PUT_NPTR(bp,NULL);
		PUT_PPTR(bp,NULL);
	}
	else if(NPTR(bp) == NULL && PPTR(bp) != NULL){
	/* CASE3 : The allocated block is last element of current list */
		
		PUT_NPTR(PPTR(bp),NULL); // set the prev block's next to NULL

		PUT_NPTR(bp,NULL);
		PUT_PPTR(bp,NULL);
	}
	else{
		/* CASE4 : The allocated block is middle of current list */

		/* Connect the original_prev and original_prev */
		PUT_NPTR(PPTR(bp), NPTR(bp));
		PUT_PPTR(NPTR(bp), PPTR(bp));

		PUT_NPTR(bp,NULL);
		PUT_PPTR(bp,NULL);
	}
}

/*
 * add_list - add block(bp) to corresponding size class list(which detemined by given block's size) 
 */
static void add_list(void *bp)
{
	size_t size = GET_SIZE(HDRP(bp));
	int i = class_idx(size);
	void *base = BPTR(i);

	if(CPTR(base) == NULL){
	/* CASE1 : size class i list is empty, set the head to bp */
		PUT_NPTR(bp, NULL);
		PUT_PPTR(bp, NULL);
		PUT_CPTR(base,bp);
	}
	else{
	/* CASE2 : size class i list has element, add bp to head of list */
		PUT_PPTR(bp, NULL);
		PUT_NPTR(bp, CPTR(base));
		PUT_PPTR(CPTR(base), bp);
		PUT_CPTR(base,bp);
	}
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
	size_t size = GET_SIZE(HDRP(ptr));
		
	PUT(HDRP(ptr), PACK(size,0));
	PUT(FTRP(ptr), PACK(size,0));
	
	PUT_NPTR(ptr,NULL);
	PUT_PPTR(ptr,NULL);

	add_list(coalesce(ptr));

}

/*
 * coalesce - coalesce the given free block(bp) to adjacent free blocks.
 */

static void *coalesce(void *bp)
{
	size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	size_t size = GET_SIZE(HDRP(bp));
	
	if(prev_alloc && next_alloc){
	/* CASE1 : both blocks are allocated */
		return bp;
	}

	else if(prev_alloc && !next_alloc){
	/* CASE2 : prev block is allocated, but next block is free block */
		remove_list(NEXT_BLKP(bp));

		size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
		PUT(HDRP(bp), PACK(size,0));
		PUT(FTRP(bp), PACK(size,0));
	}

	else if(!prev_alloc && next_alloc){
	/* CASE3 : prev block is free block, but next block is allocated */
		remove_list(PREV_BLKP(bp));

		size += GET_SIZE(HDRP(PREV_BLKP(bp)));
	
		bp = PREV_BLKP(bp);

		PUT(HDRP(bp),PACK(size,0));
		PUT(FTRP(bp),PACK(size,0));
	}
	else{
	/* CASE4 : both blocks are free block */
		remove_list(NEXT_BLKP(bp));
		remove_list(PREV_BLKP(bp));

		size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
		
		bp = PREV_BLKP(bp);

		PUT(HDRP(bp),PACK(size,0));
		PUT(FTRP(bp),PACK(size,0));
	}
	return bp;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
	void *newptr;
	void *newfreeptr;

	size_t oldsize;
	size_t restsize;
	size_t newfreesize;

	if(size == 0) mm_free(ptr);
	if(ptr == NULL) return mm_malloc(size);
	else{	
		/*Adjust block size to include overhead and alignment reqs*/
		
		if(size <= DSIZE)
			size = 2*DSIZE;
		else
			size = DSIZE * ((size + DSIZE + (DSIZE-1)) / DSIZE);

		oldsize = GET_SIZE(HDRP(ptr));

		if(oldsize<size){
		/* CASE1 : size is bigger than oldsize */
			restsize = size-oldsize;
			
			if(GET_SIZE(FTRP(ptr)+WSIZE) == 0){
			/* ptr is last block (check whether next blokc is epilogue block or not)*/
				extend_heap(restsize/WSIZE); // extend heap with restsize and merge with original block
				PUT(HDRP(ptr),PACK(oldsize + restsize,1));
				PUT(FTRP(ptr),PACK(oldsize + restsize,1));
			}
			else{
			/* otherwise, set new pointer use mm_malloc(size) and copy the content use memcpy and free original pointer */
				newptr = mm_malloc(size);
				memcpy(newptr,ptr,oldsize);
				mm_free(ptr);
				return newptr;
			}
		}
		else{
		/* CASE2 : size is smaller or equal than oldsize */
			newfreesize = oldsize-size;
			
			if(newfreesize>=3*DSIZE){
				/* if newfreesize is larger than minimum free block size(24 Bytes), split and add new free block to free list */
				PUT(HDRP(ptr),PACK(size,1));
				PUT(FTRP(ptr),PACK(size,1));	

				newfreeptr = ptr + size;

				PUT(HDRP(newfreeptr),PACK(newfreesize,0));
				PUT(FTRP(newfreeptr),PACK(newfreesize,0));

				add_list(newfreeptr);
			}
			/* if newfreesize is smaller than 24Bytes, do nothing (just allocate entire newfreesize) */
		}
		return ptr;
	}
}

/*
* mm_check - Heap Consistency Checker use raise function with SIGINT and SIGTRAP to 
*/

static int mm_check(void)
{
	int i;
	void *base;
	void *list_iter;
	void *heap_iter;
	/* Iteration for every size class free lists */
	for(i=0;i<CNUM;i++){
		base = BPTR(i);
		for(list_iter = base; list_iter != NULL; list_iter = NPTR(list_iter)){
			
			/* Is every block in the free list marked as free? */
			if(GET_ALLOC(HDRP(list_iter))){
				printf("mm_check : block 0x%x is free list but not marked as free\n",(unsigned int)list_iter);
				return 0;
			}

			/* Do the pointers in the listed free block point to vaild heap address? */
			if(!isValid(NPTR(list_iter)) || !isValid(PPTR(list_iter))){
				printf("mm_check : block 0x%x is points invalid heap address where PPTR : 0x%x and NPTR : 0x%x\n",(unsigned int)list_iter,(unsigned int)PPTR(list_iter),(unsigned int)NPTR(list_iter));
				return 0;
			}
			
			/* Do the pointers in the listed free block point to vaild free blocks? */
			if(GET_ALLOC(HDRP(NPTR(list_iter))) || GET_ALLOC(HDRP(PPTR(list_iter)))){
				printf("mm_check : block 0x%x is points invalid free blocks where PPTR : 0x%x and NPTR : 0x%x\n",(unsigned int)list_iter,(unsigned int)PPTR(list_iter),(unsigned int)NPTR(list_iter));
				return 0;
			}
		}
	}

	/* Iteration for entire heap area */
	for(heap_iter = heap_listp; GET_SIZE(HDRP(heap_iter))>0; heap_iter = NEXT_BLKP(heap_iter)){
		if(!GET_ALLOC(HDRP(heap_iter))){
			
			/* Is every free block actually in the free list? */
			if(!isListed(heap_iter)){
				printf("mm_check : block 0x%x is free but not in the free list\n",(unsigned int)heap_iter);
				return 0;
			}
			
			/* Are there any contiguous free blocks that somehow escaped coalescing?  */
			if(!GET_ALLOC(HDRP(NEXT_BLKP(heap_iter)))){
				printf("mm_check : block 0x%x and 0x%x are contiguous of free blocks\n",(unsigned int)heap_iter,(unsigned int)NEXT_BLKP(heap_iter));
				return 0;
			}
		}
	}
	
	return 1;	
}

/*
 * isListed - check whether bp is in free lists or not.
 */
static int isListed(void *bp)
{
	int i;
	void *base;
	void *list_iter;
	for(i=0;i<CNUM;i++){
		base = BPTR(i);
		for(list_iter = base; list_iter != NULL; list_iter = NPTR(list_iter)){
			if(list_iter == bp) return 1; // if bp is found in free list, return 1
		}
	}

	/* bp is not found in free lists */
	return 0;
}

/*
 * isValid - check whether bp's address is valid heap address
 */
static int isValid(void *bp)
{
	/* if bp is lower than first byte in the heap or higher than last byte in the heap, invalid */
	if(bp<mem_heap_lo() || bp>mem_heap_hi()) return 0;
	
	/* otherwise, valid */
	return 1;
}
