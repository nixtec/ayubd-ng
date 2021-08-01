/*
 * assoc_array_impl.c
 */

#include "assoc_array.h"


/*
 * last element must be NULL so that the va_* loop breaks
 */
size_t array_key_make(char *key, size_t len, const char *arg1, ...)
{
  size_t keylen;
  const char *val = NULL;
  va_list ap;

  va_start(ap, arg1);
  keylen = snprintf(key, len, "%s", arg1);
  while ((val = va_arg(ap, char *))) {
    //fprintf(stderr, "val=%s\n", val);
    /* add SUBSEP character */
    keylen += snprintf(key+keylen, len-keylen, "%c%s", SUBSEP, val);
  }
  va_end(ap);
  *(key+keylen) = '\0';

  return keylen;
}

#if 0
void array_free_walk_fn(char *key, size_t klen, void *val, void *arg)
{
  if (val) DO_FREE(val);
}

void array_free_values(array_t *a)
{
  array_walk(a, array_free_walk_fn);
}

void array_walk_impl(array_t *a, array_walk_fn_t walk_fn, uint8_t is_ord)
{
  array_iter_t *iter = NULL;
  
  if (is_ord) {
    iter = array_iter_new_ord(a);
  } else {
    iter = array_iter_new(a);
  }
  if (!iter) return;

  char *key = NULL;
  size_t klen = 0;
  void **val = NULL;

  while (!array_iter_finished(iter)) {
    key = array_iter_key(iter, &klen);
    val = array_iter_val(iter);
    walk_fn(key, klen, (char *) *val, NULL);
    array_iter_next(iter);
  }

  array_iter_free(iter);
}
#endif
