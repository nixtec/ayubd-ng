#include <stdio.h>
#include "http11_requests.h"

#define WARN_IF(EXP) fprintf(stderr, "Warning: " #EXP "\n");

int main(void)
{
  WARN_IF(Accept);
  fprintf(stderr, "%d\n", HR_HOST);

  return 0;
}
