#include "my_malloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void * createNew(size_t size) {
  mem_node_t * temp = initMem(size);
  pthread_mutex_unlock(&lock);
  return (void*)temp + sizeof(mem_node_t);
}

void* split(mem_node_t * curt, size_t size) {
  mem_node_t * temp = (mem_node_t *)((void *)curt + sizeof(mem_node_t) + size);
  size_t original_size = curt->msize;

  // update memory
  curt->msize = size;
  temp->msize = original_size - sizeof(mem_node_t) - size;

  // update connection
  mem_node_t * temp_nxt = curt->next;
  mem_node_t * temp_prv = curt->prev;
  curt->prev = NULL;
  curt->next = NULL;
  if (temp_nxt != NULL) temp_nxt->prev = temp;
  if (temp_prv != NULL) temp_prv->next = temp;
  temp->next = temp_nxt;
  temp->prev = temp_prv;

  if (head == curt) head = temp;

  pthread_mutex_unlock(&lock);
  return (void*)curt + sizeof(mem_node_t);
}

void* removeNode(mem_node_t * curt) {
  // remove it from list
  mem_node_t * myprev = curt->prev;
  mem_node_t * mynext = curt->next;
  curt->prev = NULL;
  curt->next = NULL;
  if (myprev != NULL) myprev->next = mynext;
  if (mynext != NULL) mynext->prev = myprev;

  if (head == curt) head = mynext;

  pthread_mutex_unlock(&lock);  
  return (void*)curt + sizeof(mem_node_t);
}

void merge(mem_node_t * curt) {
  mem_node_t * myprev = curt->prev;
  mem_node_t * mynext = curt->next;

  if (myprev != NULL) {
    // determine if adjecent
    if ((void*)myprev + sizeof(mem_node_t) + myprev->msize == (void*)curt) {
      myprev->msize = myprev->msize + sizeof(mem_node_t) + curt->msize;
      // update connection
      myprev->next = mynext;
      if (mynext != NULL) mynext->prev = myprev;
      curt = myprev;
    }
  }

  if (mynext != NULL) {
    // determine if adjecent
    if ((void*)curt + sizeof(mem_node_t) + curt->msize == (void*)mynext) {
      //update memory
      curt->msize = curt->msize + sizeof(mem_node_t) + mynext->msize;
      //update connection
      curt->next = mynext->next;
      if (mynext->next != NULL) mynext->next->prev = curt;
    }
  }
}

mem_node_t * initMem(size_t size) {
  mem_node_t * curt = NULL;

  // allocate memory by moving program break
  curt = sbrk(size + sizeof(mem_node_t));
  curt->prev = NULL;
  curt->next = NULL;
  curt->msize = size;
  
  return curt;
}

void *ts_malloc_lock(size_t size) {
  pthread_mutex_lock(&lock);
  if (head == NULL) head = initMem(size);

  mem_node_t * curt = head;
  mem_node_t * best = NULL;

  while (curt != NULL) {
    if (curt->msize > size + sizeof(mem_node_t)) {
      if (best == NULL) best = curt;
      if (best->msize > curt->msize) best = curt;
    } 
    
    if (curt->msize == size) {
      best = curt;
      break;
    }

    curt = curt->next;
  }

  // no matches
  if (best == NULL) return createNew(size);

  if (best->msize == size) {  // memory perfectly matches
    return removeNode(best);
  }
  else {  // memory with remaining
    return split(best, size);
  }
}

void ts_free_lock(void *ptr) {
  pthread_mutex_lock(&lock);
  
  mem_node_t * curt = ptr;
  curt--;

  if (curt->next != NULL) printf("next is not null!\n");
  if (curt->prev != NULL) printf("prev is not null!\n");
  
  if (head == NULL) {
    head = curt;
    pthread_mutex_unlock(&lock);
    return;
  }

  // find front nearest in freelist
  mem_node_t * front = head;
  mem_node_t * end = head->next;
  mem_node_t * end_prv = head;

  // all big
  if (front > curt) {
    curt->next = front;
    front->prev = curt;
    head = curt;
    merge(curt);
    pthread_mutex_unlock(&lock);
    return;
  }

  // find big
  while (end != NULL) {
    if (end > curt && end_prv < curt) break;
    end_prv = end;
    end = end->next;
  }

  // all small
  if (end == NULL) {
    end_prv->next = curt;
    curt->prev = end_prv;
    merge(curt);
    pthread_mutex_unlock(&lock);
    return;
  }

  // small exist
  while (front != NULL && front->next != NULL) {
    if (front < curt && front->next > curt) {
      break;
    }
    front = front->next;
  }

  // both exist
  front->next = curt;
  end->prev = curt;
  curt->next = end;
  curt->prev = front;
  merge(curt);
  pthread_mutex_unlock(&lock);
}


/*
    special functions for nolock
*/




mem_node_t * tinitMem(size_t size) {
  mem_node_t * curt = NULL;

  // allocate memory by moving program break
  pthread_mutex_lock(&lock);
  curt = sbrk(size + sizeof(mem_node_t));
  pthread_mutex_unlock(&lock);
  curt->prev = NULL;
  curt->next = NULL;
  curt->msize = size;
  
  return curt;
}

void * tcreateNew(size_t size) {
  mem_node_t * temp = tinitMem(size);
  return (void*)temp + sizeof(mem_node_t);
}

void* tsplit(mem_node_t * curt, size_t size) {
  mem_node_t * temp = (mem_node_t *)((void *)curt + sizeof(mem_node_t) + size);
  size_t original_size = curt->msize;

  // update memory
  curt->msize = size;
  temp->msize = original_size - sizeof(mem_node_t) - size;

  // update connection
  mem_node_t * temp_nxt = curt->next;
  mem_node_t * temp_prv = curt->prev;
  curt->prev = NULL;
  curt->next = NULL;
  if (temp_nxt != NULL) temp_nxt->prev = temp;
  if (temp_prv != NULL) temp_prv->next = temp;
  temp->next = temp_nxt;
  temp->prev = temp_prv;

  if (thead == curt) thead = temp;

  return (void*)curt + sizeof(mem_node_t);
}

void* tremoveNode(mem_node_t * curt) {
  // remove it from list
  mem_node_t * myprev = curt->prev;
  mem_node_t * mynext = curt->next;
  curt->prev = NULL;
  curt->next = NULL;
  if (myprev != NULL) myprev->next = mynext;
  if (mynext != NULL) mynext->prev = myprev;

  if (thead == curt) thead = mynext;
 
  return (void*)curt + sizeof(mem_node_t);
}


void *ts_malloc_nolock(size_t size) {
  if (thead == NULL) thead = tinitMem(size);

  mem_node_t * curt = thead;
  mem_node_t * best = NULL;

  while (curt != NULL) {
    if (curt->msize > size + sizeof(mem_node_t)) {
      if (best == NULL) best = curt;
      if (best->msize > curt->msize) best = curt;
    } 
    
    if (curt->msize == size) {
      best = curt;
      break;
    }

    curt = curt->next;
  }

  // no matches
  if (best == NULL) return tcreateNew(size);

  if (best->msize == size) {  // memory perfectly matches
    return tremoveNode(best);
  }
  else {  // memory with remaining
    return tsplit(best, size);
  }
}


void ts_free_nolock(void *ptr) {
  mem_node_t * curt = ptr;
  curt--;

  if (curt->next != NULL) printf("next is not null!\n");
  if (curt->prev != NULL) printf("prev is not null!\n");
  
  if (thead == NULL) {
    thead = curt;
    return;
  }

  // find front nearest in freelist
  mem_node_t * front = thead;
  mem_node_t * end = thead->next;
  mem_node_t * end_prv = thead;

  // all big
  if (front > curt) {
    curt->next = front;
    front->prev = curt;
    thead = curt;
    merge(curt);
    return;
  }

  // find big
  while (end != NULL) {
    if (end > curt && end_prv < curt) break;
    end_prv = end;
    end = end->next;
  }

  // all small
  if (end == NULL) {
    end_prv->next = curt;
    curt->prev = end_prv;
    merge(curt);
    return;
  }

  // small exist
  while (front != NULL && front->next != NULL) {
    if (front < curt && front->next > curt) {
      break;
    }
    front = front->next;
  }

  // both exist
  front->next = curt;
  end->prev = curt;
  curt->next = end;
  curt->prev = front;
  merge(curt);
}
