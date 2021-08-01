#include <stdio.h>
#include <threads.h>
#include <stdatomic.h>

enum CONSTANTS {
  NUM_THREADS = 1000,
  NUM_ITERS = 1000
};

_Atomic int acnt;
int cnt;

int f(void* thr_data)
{
  int n;
  for (n = 0; n < NUM_ITERS; ++n) {
    ++cnt;
    ++acnt;
    // for this example, relaxed memory order is sufficient, e.g.
    // atomic_fetch_add_explicit(&acnt, 1, memory_order_relaxed);
  }
  return 0;
}

int main(void)
{
  thrd_t thr[NUM_THREADS];
  int n;
  for(n = 0; n < NUM_THREADS; ++n)
    thrd_create(&thr[n], f, NULL);
  for(n = 0; n < NUM_THREADS; ++n)
    thrd_join(thr[n], NULL);

  printf("The atomic counter is %u\n", acnt);
  printf("The non-atomic counter is %u\n", cnt);

  return 0;
}
