/*
 * ioqueue.c
 */
#include "ioqueue.h"

ioqueue_t *ioqueue_new(int flags)
{
  ioqueue_t *iq = DO_CALLOC(1, sizeof(ioqueue_t));
  if (iq) {
    iq->flags = flags;
  }
  return iq;
}

void ioqueue_set_flags(ioqueue_t *iq, int flags)
{
  assert(iq != NULL);
  iq->flags |= flags;
}

