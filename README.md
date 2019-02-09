# thread-safe-malloc
An implementation of thread-safe malloc library.

Two different thread-safe versions (i.e. safe for concurrent access by different threads of a process) of the malloc() and free() functions are implemented here. Both of
the thread-safe malloc and free functions use the best fit allocation policy.

Version 1 uses lock-based synchronization to prevent race conditions that would lead to incorrect results. These functions are to be named:

```
//Thread Safe malloc/free: locking version
void *ts_malloc_lock(size_t size);
void ts_free_lock(void *ptr);
```

Version 2 is a non-lock based synchronization which uses Thread-Local Storage.

```
//Thread Safe malloc/free: non-locking version
void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);
```

A **Makefile** is provided to generate "libmymalloc.so". Hence other programs can link against "libmymalloc.so" (-lmymalloc), and then have access to the new malloc functions.
