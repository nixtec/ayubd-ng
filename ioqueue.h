/*
 * ioqueue.h
 */
#ifndef __IOQUEUE_H__
#define __IOQUEUE_H__

#include "main.h"

#define IOQUEUE_CHUNK IOV_CHUNK

typedef struct ioqueue_struct ioqueue_t;

typedef int (*ioqueue_func_t)(ioqueue_t *iq, array_t *args);
struct ioqueue_struct {
  int flags;
  int *fd;
  int fdlen;
};

/* if return value is 0, nothing to be done, stay silent
 * if return value is more than 0, take to be taken from 'data' and put to output
 * if negative value, may be used for special purposes, like sendfile() and other things (not yet thought)
 */

ioqueue_t *ioqueue_new(int flags);
void ioqueue_set_flags(ioqueue_t *iq, int flags);


#endif
