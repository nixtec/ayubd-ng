#include <stdio.h>
#include <threads.h>
#include <stdatomic.h>
#include <stdio.h>
#include <lfq.h>

enum CONSTANTS {
  NUM_THREADS = 1,
  NUM_ITERS = 1000
};

_Atomic int acnt, qcnt;
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


int producer(void *data)
{
  struct lfq *q = data;


  for (int n = 0; n < NUM_ITERS; ++n) {
    ++cnt;
    ++acnt;
    ++qcnt;
    lfq_enqueue(q, (void *) qcnt);
  }

  return 0;
}

int consumer(void *data)
{
  struct lfq *q = data;
  int xx;

  for (int n = 0; n < NUM_ITERS; ++n) {
    --cnt;
    --acnt;
    xx = (int) lfq_dequeue(q);
    if (xx == 0) { fprintf(stderr, "No more elements\n"); }
    else {
      --qcnt;
    }
  }

  return 0;
}

int main(void)
{
  thrd_t thr[NUM_THREADS*2];
  int n;
  struct lfq *q = lfq_new();

  for(n = 0; n < NUM_THREADS; ++n)
    thrd_create(&thr[n], producer, q);
  for(; n < NUM_THREADS*2; ++n)
    thrd_create(&thr[n], consumer, q);

  for(n = 0; n < NUM_THREADS*2; ++n)
    thrd_join(thr[n], NULL);

  printf("The non-atomic counter is %u\n", cnt);
  printf("The atomic counter is %u\n", acnt);
  printf("The Queue counter is %u\n", qcnt);

  return 0;
}
