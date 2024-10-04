#ifndef __MEMORY_H__
#define __MEMORY_H__

#define USE_MEM_DEBUG		1
#define LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT	0
#define MEM_ALIGNMENT       4
#define USE_MEM_ASSERT		0

/* MEM_SIZE would have to be aligned, but using 64000 here instead of
 * 65535 leaves some room for alignment...
 */

typedef unsigned int mem_size_t;


typedef unsigned int mem_ptr_t;

//void  mem_init(void);
//void *mem_calloc_ex(const char *name, mem_size_t count, mem_size_t size);
//void *mem_malloc_ex(const char *name, mem_size_t size);
//#define mem_malloc(SIZE)	mem_malloc_ex(__FUNCTION__, (SIZE))
#define mem_init()

void sys_meminit(void *pool, unsigned int size);

#if USE_MEM_DEBUG

void *mem_malloc_ex(const char *name, mem_size_t size);
void *mem_calloc_ex(const char *name, mem_size_t count, mem_size_t size);

#define mem_malloc(SIZE) mem_malloc_ex(__FUNCTION__, SIZE)
#define mem_calloc(SIZE, COUNT) mem_calloc_ex(__FUNCTION__, (SIZE), (COUNT))

#else

void *mem_malloc(mem_size_t size);
void *mem_calloc(mem_size_t count, mem_size_t size);
#endif

#define malloc 		mem_malloc
#define free 		mem_free

void mem_slide_check(int show_list);

uint32_t mem_free(void *rmem);
uint32_t get_mem_size(void *rmem);
#endif /*__LWIP_MEM_H__*/
