/*
 * lfq.c
 * lock-free queue
 * safe for one producer, one consumer
 * Idea from Dr. Dobb's Journal
 * One thread can write and another read -- at the same time! http://www.drdobbs.com/parallel/lock-free-queues/208801974
 * Writing Lock-Free Code: A Corrected Queue: http://www.drdobbs.com/parallel/writing-lock-free-code-a-corrected-queue/210604448
 * no locking is used, the environment/system will ensure that one producer and one consumer are using the same queue at a time
 */
#include "lfq.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct lfq_node *lfq_node_new(void *value)
{
  struct lfq_node *node = DO_MALLOC(sizeof(*node));
  node->value = value;
  node->link = NULL;

  return node;
}

void lfq_node_delete(struct lfq_node *node)
{
  DO_FREE(node);
}

struct lfq *lfq_new(void)
{
  struct lfq *q = DO_MALLOC(sizeof(struct lfq));
  struct lfq_node *node = lfq_node_new(NULL);
  q->first = q->divider = q->last = node;

  return q;
}

/*
 * destroy the queue
 */
void lfq_delete(struct lfq *q)
{
  struct lfq_node *tmp = NULL;
  while (q->first != NULL) {
    tmp = q->first;
    q->first = tmp->link;
    DO_FREE(tmp);
  }

  DO_FREE(q);
}


void lfq_in(struct lfq *q, void *value)
{
  struct lfq_node *tmp = NULL;
  q->last->link = lfq_node_new(value); /* add the new item */
  q->last = q->last->link; /* publish it */

  while (q->first != q->divider) { /* trim unused nodes */
    tmp = q->first;
    q->first = q->first->link;
    DO_FREE(tmp);
  }
}

/*
 * dequeue doesn't remove the node from Q
 * it just advances the divider
 */
void *lfq_out(struct lfq *q)
{
  void *result = NULL;
  if (q->divider != q->last) {
    result = q->divider->link->value;
    q->divider = q->divider->link;
    return result;
  }
  return NULL;
}

int lfq_is_empty(struct lfq *q)
{
  return (q->divider == q->last);
}

