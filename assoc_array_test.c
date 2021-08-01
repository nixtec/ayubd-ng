/*
 * assoc_array_test.c
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "assoc_array.h"


void array_walk_print(char *key, size_t klen, void *val, void *arg)
{
  printf("key: <%s>, val: <%s>\n", key, (char *) val);
}

int main(int argc, char *argv[])
{
  array_t *a = array_new();
  if (!a) exit(EXIT_FAILURE);

  char buf[8192];
  int idx = 0;
  char key[20];
  size_t key_len = 0;

  while (fgets(buf, sizeof(buf), stdin)) {
    idx++;
    key_len = sprintf(key, "%d", idx);
    array_set(a, key, key_len+1, (void *) strdup(buf));
    //array_set(a, key, key_len+1, (void *) buf);
  }
  printf("Stored keys: %lu, Size: %lu\n", array_size(a), array_meminfo(a));

  //array_walk(a, array_walk_print);
  //array_walk_ord(a, array_walk_print);


  array_free(a);

  return 0;
}


