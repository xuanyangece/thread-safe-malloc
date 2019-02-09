#include <stdlib.h>
#include <pthread.h>

typedef struct mem_node mem_node_t;
struct mem_node {
  mem_node_t *prev;
  mem_node_t *next;
  size_t msize;
};

// for the lock
mem_node_t * head = NULL;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// for the nolock
__thread mem_node_t * thead = NULL;

mem_node_t * initMem(size_t size);

//Thread Safe malloc/free: locking version
void *ts_malloc_lock(size_t size);
void ts_free_lock(void *ptr);

//Thread Safe malloc/free: non-locking version
void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);
