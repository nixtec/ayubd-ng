/*
 * stack.h
 */
#ifndef __STACK_H__
#define __STACK_H__

#include "config.h"
#include "node.h"

typedef struct {
  struct node *top;
} stack_t;

/* double pointer because the reference may change upon insertion */
stack_t *stack_new(void);
void stack_in(stack_t *st, void *val);
void *stack_out(stack_t *st);
void stack_delete(stack_t *st);

#endif
