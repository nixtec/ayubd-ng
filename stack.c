/*
 * stack.c
 */
#include "stack.h"
#include <assert.h>

stack_t *stack_new(void)
{
  stack_t *ret = DO_MALLOC(sizeof(*ret));
  assert(ret != NULL);
  ret->top = NULL;

  return ret;
}

void stack_in(stack_t *st, void *val)
{
  assert(st != NULL);
  struct node *tmp = DO_MALLOC(sizeof(*tmp));
  tmp->value = val;
  tmp->link = st->top;
  st->top = tmp;
}

void *stack_out(stack_t *st)
{
  assert(st != NULL);
  if (st->top == NULL) return NULL;

  void *val = NULL;
  struct node *tmp = st->top;
  st->top = tmp->link;
  val = tmp->value;
  DO_FREE(tmp);

  return val;
}

void stack_delete(stack_t *st)
{
  assert(st != NULL);
  struct node *tmp = st->top;

  while (st->top) {
    tmp = st->top;
    st->top = tmp->link;
    DO_FREE(tmp);
  }
  DO_FREE(st);
}
