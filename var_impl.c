/*
 * var_impl.c
 * implementation of 'var' type
 * so that high-level code like AWK can be done in C
 */

#define VAR_LEN 8
typedef struct {
  size_t var_size;
  uint8_t var_type;
  union {
    char c[VAR_LEN];
    int64_t i;
    double d;
    char *s;
  } var_var;
  void (*free_func)(void *ptr); /* free function for string type data */
} var_t;

var_t *var_new(void)
{
  var_t *v = calloc(sizeof(var_t));
  if (v) {
    v->free_func = free;
  }
}

int main(void)
{

  return 0;
}
