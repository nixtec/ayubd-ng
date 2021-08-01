/*
 * lfq.h
 * Lock-Free Queue Implementation (for single producer, single consumer, not locking required)
 */
#ifndef __LFQ_H__
#define __LFQ_H__

#include "config.h"

struct lfq_node {
  void *value;
  struct lfq_node *link;
};


struct lfq {
  struct lfq_node *first; /* for producer only */
#if 0
  volatile struct lfq_node *divider, *last; /* shared */
#endif
  struct lfq_node *divider, *last; /* shared */
};

struct lfq_node *lfq_node_new(void *value);
void lfq_node_delete(struct lfq_node *node);
struct lfq *lfq_new(void);
void lfq_in(struct lfq *q, void *value);
void *lfq_out(struct lfq *q);
void lfq_delete(struct lfq *q);
#define lfq_destroy(q) lfq_delete(q)
int lfq_is_empty(struct lfq *q);

#endif
