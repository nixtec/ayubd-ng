/*
 * main.h
 */
#ifndef __MAIN_H__
#define __MAIN_H__

#define _GNU_SOURCE


#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>		/* nanosleep(2), etc. */
#include <setjmp.h>
#include <dlfcn.h>
#include "defines.h"		/* macro definitions */
#include "poll.h"
//#include "lfq.h"		/* lock free queue (thread-safe for one producer and one consumer) */
// lfq.* not required, as I have implemented the same using sys/queue.h macros (STAILQ_*)
#include "assoc_array.h"
#include "utils.h"
#include <sys/queue.h>		/* various types of list-based data structures (stack, queue, etc.) */

#include <sys/socket.h>
//#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netdb.h>

#ifdef USE_PROG
#include "prog.h"
#endif

#ifdef USE_CORO
#include "libaco/aco.h"
#endif

#ifdef USE_REDIS
#include <hiredis.h>
#endif

#ifndef __STDC_NO_ATOMICS__
#include <stdatomic.h>
#endif

#ifdef USE_RAND
#include <sys/random.h>
#ifndef PRNGBUFSZ
#define PRNGBUFSZ 128
#endif
#endif

#ifdef USE_HS
#include <hs.h>
#endif

//#include <atomic_hash.h>


#define err_exit(msg) \
  do { perror(msg); exit(EXIT_FAILURE); } while (0)
#define err_warn(msg) perror(msg)

#define USLEEP(x) do { \
  struct timespec __ts__; \
  __ts__.tv_sec = x / 1000; \
  __ts__.tv_nsec = (x % 1000) * 1000; \
  nanosleep(&__ts__, NULL); \
} while (0)

typedef jmp_buf state_t;
#define state_new() calloc(1, sizeof(state_t))
#define state_save(state) setjmp(state)
#define state_restore(state) longjmp(state,1)


#ifndef CHECKFLAG
#define CHECKFLAG(x,y) ((x) & (y))
#endif
#ifndef SETFLAGS
#define SETFLAGS(fd,flags) fcntl((fd), F_SETFL, fcntl((fd), F_GETFL, 0) | (flags))
#endif
#ifndef SETNONBLOCKING
#define SETNONBLOCKING(fd) SETFLAGS(fd, O_NONBLOCK)
#endif

#ifndef MAXEVENTS
#define MAXEVENTS 1024
#endif

#ifdef __linux__
#define ACCEPT(sfd,addr,len) accept4((sfd),(addr),(len), SOCK_NONBLOCK | SOCK_CLOEXEC)
#else
#define ACCEPT(sfd,addr,len) accept((sfd),(addr),(len))
#endif


/* Stack */
SLIST_HEAD(slist, slist_node);
struct slist_node {
  void *val;
  SLIST_ENTRY(slist_node) slist_link;
};

/* Queue */
STAILQ_HEAD(stailq, stailq_node);
struct stailq_node {
  void *val; /* value pointer */
//  struct stailq_node *divider; /* to make it lock-free (for single producer and single consumer), we use a divider */
  STAILQ_ENTRY(stailq_node) stailq_link;
};

/* use udata pointer array for various purposes */
#define UD_CORE_SYSPTR	1
#define UD_CORE_NREAD	2
#define UD_CORE_NUM	3
#define UD_CORE_MSGHDR	4
#define UD_MOD_HTTP_ROUTE	5	/* usually required for mod_http */
#define UD_RESERVED	8 /* after this modules can use extensively for their own use */
#define UDATASZ		16 /* maximum size of UDATA (number of pointers can be stored) */



/* usually hook_func_t takes pointer to self and sys */
typedef int (*hook_func_t)(void *);
typedef struct {
  struct slist *mp; /* memory pool, all allocated memory will be referenced here, so that upon hook_free() we can free all */
  int fd;
  hook_func_t ready; /* invoked by main thread, to detect abnormal signals or file closing, to save context switches for small tasks */
  hook_func_t process;
  array_t *data; /* user data (session, etc.), modules will use it extensively for protocol-dependent way (can be used for hacks) */
  event_t ev; /* event specification (used internally) */
#ifdef __STDC_NO_ATOMICS__
#warning NO ATOMIC OPS SUPPORT BUILT-IN
#else
  _Atomic
#endif
  int refcnt; /* reference counting, if hits 0, call hook_free_main() to do real free */
#ifdef USE_CORO
  aco_t *co;
  /* exploit t he following for various purposes (mainly for coroutines) */
#endif
  unsigned long int udata[UDATASZ]; /* user data pointer (usually system_t *), modules will use it to pass data handler (for now) */
} hook_t;

#define UDATA_DUP(s,d) memcpy((d)->udata, (s)->udata, UDATASZ*sizeof(unsigned long int))

/*
 * as of gcc 4.8.2, __sync* built-ins have been deprecated in favor of the __atomic built-ins
 * https://gcc.gnu.org/onlinedocs/gcc-4.8.2/gcc/_005f_005fatomic-Builtins.html
 */

/*
 * memory model to be set can be found in following:
 * https://gcc.gnu.org/onlinedocs/gcc-4.8.2/gcc/_005f_005fatomic-Builtins.html
 * __ATOMIC_RELAXED
 * No barriers or synchronization. 
 * __ATOMIC_CONSUME
 * Data dependency only for both barrier and synchronization with another thread. 
 * __ATOMIC_ACQUIRE
 * Barrier to hoisting of code and synchronizes with release (or stronger) semantic stores from another thread. 
 * __ATOMIC_RELEASE
 * Barrier to sinking of code and synchronizes with acquire (or stronger) semantic loads from another thread. 
 * __ATOMIC_ACQ_REL
 * Full barrier in both directions and synchronizes with acquire loads and release stores in another thread. 
 * __ATOMIC_SEQ_CST
 * Full barrier in both directions and synchronizes with acquire loads and release stores in all threads.
 */
#define ATOMIC_MEMMODEL __ATOMIC_SEQ_CST


/* LIFO (Stack) Implementation: insert to head, remove from head */
#define slist_new(p) do { (p) = DO_MALLOC(sizeof(*(p))); SLIST_INIT((p)); } while (0)
#define slist_in(p,v) do { struct slist_node *np = DO_MALLOC(sizeof(*np)); np->val = (void *) (v); SLIST_INSERT_HEAD((p), np, slist_link); } while (0)
#define slist_out(p,v) do { struct slist_node *np = SLIST_FIRST((p)); if (!np) { v = NULL; } else { v = (typeof(v)) np->val; SLIST_REMOVE_HEAD((p), slist_link); DO_FREE(np); } } while (0)
/* if 'vfree' (value free) is 1, then free the 'val' of 'node' */
#define slist_free(p,vfree) do { struct slist_node *np; while ((np = SLIST_FIRST((p)))) { SLIST_REMOVE_HEAD((p), slist_link); if ((vfree) && np->val) DO_FREE(np->val); DO_FREE(np); }; DO_FREE((p)); } while (0)
/* remove node which contains 'val' equal to 'v' */
#define slist_remove_val(p,v,vfree) do { struct slist_node *np; SLIST_FOREACH(np, (p), slist_link) { if (np->val == (v)) { SLIST_REMOVE((p), np, slist_node, slist_link); if (vfree) DO_FREE(np->val); DO_FREE(np); break; } } } while (0)

/* FIFO (Queue) Implementation: insert to tail, remove from head */
#define stailq_new(p) do { (p) = DO_MALLOC(sizeof(*(p))); STAILQ_INIT((p)); } while (0)
#define stailq_in(p,v) do { struct stailq_node *np = DO_MALLOC(sizeof(*np)); np->val = (v); STAILQ_INSERT_TAIL((p), np, stailq_link); } while (0)
#define stailq_out(p,v) do { struct stailq_node *np = STAILQ_FIRST((p)); if (!np) { v = NULL; } else { v = np->val; STAILQ_REMOVE_HEAD((p), stailq_link); DO_FREE(np); } } while (0)
#define stailq_free(p,vfree) do { struct stailq_node *np; while ((np = STAILQ_FIRST((p)))) { STAILQ_REMOVE_HEAD((p), stailq_link); if ((vfree) && np->val) DO_FREE(np->val); DO_FREE(np); }; DO_FREE((p)); } while (0)


/* for lock-free version (single producer, single consumer, pass 'd' which is (** struct stailq_node) [like a tail referernce] */
/* d = divider node, 'val' points to the node in actual queue */
/* uses hack to find 'last' node directly, so we need to know the name of last node variable (stqh_last) */
/* for details of structure please have a look at /usr/include/sys/queue.h */
#define STAILQ_TAIL_REF(p) (*(p)->stqh_last)
#define stailq_new_lf_spsc(p,d) do { (p) = DO_MALLOC(sizeof(*(p))); STAILQ_INIT((p)); (d) = DO_MALLOC(sizeof(*(d))); STAILQ_INSERT_TAIL((p), (d), stailq_link); /* now 'first', 'last', 'divider' all points to first (dummy) node */ } while (0)
#define stailq_trim_lf_spsc(p,d) do { struct stailq_node *tnp; while ((tnp = STAILQ_FIRST((p))) != (d)) { STAILQ_REMOVE_HEAD((p), stailq_link); DO_FREE(tnp); } } while (0)
#define stailq_in_lf_spsc(p,d,v) do { struct stailq_node *np = DO_MALLOC(sizeof(*np)); np->val = (v); STAILQ_INSERT_TAIL((p), np, stailq_link); stailq_trim_lf_spsc((p), (d)); } while (0)
#define stailq_empty_lf_spsc(p,d) ((d) == STAILQ_TAIL_REF((p)))
#define stailq_out_lf_spsc(p,d,v) do { if (!stailq_empty_lf_spsc((p), (d))) { (d) = STAILQ_NEXT((d), stailq_link); (v) = (d)->val; } else { (v) = NULL; } } while (0)
/* if 'vfree' (value free) is 1, then free the 'val' of 'node' */
/* (d) holds a dummy node in queue, upon queue free, it will be freed as well */
#define stailq_free_lf_spsc(p,d,vfree) stailq_free((p), (vfree))

#define DO_IOCALL(n,...) do { (n) = __VA_ARGS__; } while ((n) < 0 && errno == EINTR)


#define hook_new(h)				\
	do {					\
	  (h) = DO_CALLOC(1, sizeof(*(h)));	\
	  if (h) {				\
	    (h)->refcnt = 1;			\
	    slist_new((h)->mp);			\
	  }					\
	} while (0)
/* hook_*alloc functions might require locking, it's not lock free for SPSC */
#ifdef USE_CORO
#define hook_exec(h,x,fn,p) do { if ((h)->co) aco_resume((h)->co); else aco_resume((h)->co = aco_create(((system_t *)(h)->udata)->main_co[(x)], ((system_t *)(h)->udata)->main_co_sstk[(x)], 0, (fn), (p))); } while (0)
#else
#define hook_exec(h,x,fn,p) ((fn)((p)))
#endif

/* having side effects of multiple evaluation (use with care) */
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define IOV_MALLOC(p) do { (p)->iov_len = IOV_CHUNK; (p)->iov_base = DO_MALLOC(IOV_CHUNK); } while (0)
#define IOV_ZALLOC(p) do { (p)->iov_len = IOV_CHUNK; (p)->iov_base = DO_CALLOC(1, IOV_CHUNK); } while (0)
//#define IOV_PRINTF(p,fmt,...) do { (p)->iov_len = MIN(snprintf((p)->iov_base, IOV_CHUNK, (fmt), __VA_ARGS__), IOV_CHUNK);; (p)->iov_len = MIN(ix, IOV_CHUNK); } while (0)
#define IOV_PRINTF(p,fmt,...) ((p)->iov_len = MIN(snprintf((p)->iov_base, IOV_CHUNK, (fmt), __VA_ARGS__), IOV_CHUNK))
#define IOV_RESET(p) ((p)->iov_len = IOV_CHUNK)



#define hook_add(h,p) slist_in((h)->mp,(p))
#define hook_malloc(h,p,sz) do { (p) = (typeof((p))) DO_MALLOC((sz)); hook_add((h), (p)); } while (0)
#define hook_malloc_iov(h,p) do { IOV_MALLOC((p)); hook_add((h), (p)->iov_base); } while (0)
#define hook_calloc_internal(h,p,n,sz) do { (p) = (typeof((p))) DO_CALLOC((n), (sz)); hook_add((h), (p)); } while (0)
#define hook_zalloc(h,p,sz) hook_calloc_internal((h),(p),1,(sz))
#define hook_zalloc_iov(h,p) do { IOV_ZALLOC((p)); hook_add((h), (p)->iov_base); } while (0)
/* 'vfree=0' in slist_remove_val() because 'realloc' already will free old memory if memory location changes */
//#define hook_realloc(h,p,sz) do { typeof((p)) tmp = DO_REALLOC((p), (sz)); assert(tmp != NULL); if (tmp != (p)) { slist_remove_val((h)->mp, (p), 0); }; (p) = tmp; hook_add((h), (p)); } while (0)
#ifdef USE_CORO
#define hook_free_real(h)			\
	do {					\
	  slist_free((h)->mp, 1);		\
	  if ((h)->data) array_free((h)->data);	\
	  if ((h)->co) aco_destroy((h)->co);	\
	  DO_FREE((h));				\
	  (h) = NULL;				\
	} while (0)
#else
#define hook_free_real(h)			\
	do {					\
	  slist_free((h)->mp, 1);		\
	  if ((h)->data) array_free((h)->data);	\
	  DO_FREE((h));				\
	  (h) = NULL;				\
	} while (0)
#endif


#ifdef __STDC_NO_ATOMICS__
#define ATOMIC_ADD(v,n) (__atomic_add_fetch((int *) &(v), (n), ATOMIC_MEMMODEL))
#define ATOMIC_SUB(v,n) (__atomic_sub_fetch((int *) &(v), (n), ATOMIC_MEMMODEL))
#else
#define ATOMIC_ADD(v,n) ((v) += (n))
#define ATOMIC_SUB(v,n) ((v) -= (n))
#endif


#define hook_free(ptr) do { if (ATOMIC_SUB((ptr)->refcnt,1) == 0) hook_free_real((ptr)); } while (0)
#define hook_ref(ptr) ATOMIC_ADD((ptr)->refcnt, 1)
#define hook_unref(ptr) hook_free((ptr))
#define hook_close(ptr) do { close((ptr)->fd); hook_free((ptr)); } while (0)

#define SYMTBLSZ (1<<4)
#define NR_SYS_NWORKERS	1 /* sys.nworkers */
#define NR_SYS_NEVENTS	2 /* sys.nevents */
#define NR_SYS_SYSPTR	3 /* system_t * */

typedef struct {
  int eventfd; /* main asynchronous event descriptor */
  //struct lfq **q; /* workers' queue (queue per worker) */
  struct stailq **q; /* workers' queue (queue per worker) */
  struct stailq_node **qdiv; /* divider for each queue to make it lock free */
  int nworkers; /* number of worker threads */
  sem_t *sem; /* sem per worker */
  event_t *events; /* list of events */
  int nevents; /* number of events to be fetched in one call */

#ifdef __STDC_NO_ATOMICS__
#warning NO ATOMIC OPS SUPPORT BUILT-IN
#else
  _Atomic
#endif
  int nsessions; /* reference counting, if hits 0, call hook_free_main() to do real free */

#ifdef USE_LINUX_AIO
  aio_context_t ioctx;
  struct iocb **iocb;
  struct stailq **ioq; /* per worker queue for I/O */
  struct stailq_node **ioqdiv; /* divider for each queue to make it lock free */
#endif

#ifdef USE_CORO
//#warning Using COROUTINE
  aco_t **main_co;
  aco_share_stack_t **main_co_sstk;
#else
//#warning *NOT* Using COROUTINE
#endif

#ifdef USE_REDIS
  redisContext **redis_ctx; /* redis context */
  redisReply **redis_reply; /* redis reply object */
#endif

#ifdef USE_RAND
  struct random_data *rand_state;
  char **rand_statebuf;
#endif

#ifdef USE_ATOMIC_HASH
  //hash_t *hash; /* this is lock-free atomic hash table, multiple threads can perform operations at a time */
#endif

  unsigned long int *symtbl;
  int nsymbols;
  //array_t *symtbl; /* global symbol table */

#ifdef USE_HS
  hs_database_t **hs_database;
  hs_scratch_t **hs_scratch;
#endif

} system_t;

#define RANDOM(h,v) random_r(&(((system_t *) ((h)->udata[UD_CORE_SYSPTR]))->rand_state[(h)->udata[UD_CORE_NUM]]), &(v))


#if 0
/* parameter to be passed to worker threads upon creation */
struct winfo {
  system_t *sys;
  int workernum;
};
#endif


system_t *ayubd_init(array_t *config);
array_t *ayubd_config_read(const char *cfile);
int ayubd_module_load(system_t *sys, char *mod_name);
int ayubd_run(system_t *sys);
void ayubd_quit(system_t *sys);
void *ayubd_worker(void *arg); /* worker thread entry point */

#endif
