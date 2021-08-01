#include <stdio.h>

int main(void)
{
  int x = 4;

  printf("%d\n", (x += 5));
  fprintf(stderr, "Size of int = %lu\n", sizeof(int));
  fprintf(stderr, "Size of long = %lu\n", sizeof(long));
  fprintf(stderr, "Size of float = %lu\n", sizeof(float));
  fprintf(stderr, "Size of double = %lu\n", sizeof(double));

  return 0;
}
