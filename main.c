/*
 * main.c
 */
#include "main.h"
#include "module.h"
#include "signal.h"

#define MODULE_DIR "modules"
#define MODULE_DEP "modules.dep"



#if 0
#include <malloc.h>


//void *old_malloc_hook, *old_realloc_hook, *old_free_hook;

void *malloc_hook_f(size_t sz, const void *paddr)
{
  return DO_MALLOC(sz);
}


void *realloc_hook_f(void *p, size_t sz, const void *paddr)
{
  return DO_REALLOC(p, sz);
}

void *free_hook_f(void *p, const void *paddr)
{
  return DO_FREE(p);
}




void init_malloc_debug(void)
{
  __malloc_hook = malloc_hook_f;
  __realloc_hook = realloc_hook_f;
  __free_hook = free_hook_f;
}


#endif







/* do your experiments on various things (DS, etc.) here */
int svc_core_detect_system(void)
{
  char *key = DO_CALLOC(1, ARRAY_KEY_MAX);
  size_t keylen, skeylen;
  array_t *info = array_new();
  skeylen = ARRAY_KEY_MAX;
  keylen = array_key_make(key, skeylen, "list", "0", NULL);
  printf("key=%s, len=%ld\n", key, keylen);
  _aset(info, "open_max", sysconf(_SC_OPEN_MAX));
  _aset(info, "cpu_core", sysconf(_SC_NPROCESSORS_ONLN));
  _aset(info, "page_size", sysconf(_SC_PAGE_SIZE));
  _aset(info, "available_pages", sysconf(_SC_AVPHYS_PAGES));
  _aset(info, "hello", "Hello World"); /* to make it usable outside of this function you must strdup() it */

  fprintf(stderr, "Open Max: %ld\n", _aget(info, "open_max"));
  fprintf(stderr, "CPU Core: %ld\n", _aget(info, "cpu_core"));
  fprintf(stderr, "Page Size: %ld\n", _aget(info, "page_size"));
  fprintf(stderr, "Available Pages: %ld\n", _aget(info, "available_pages"));
  fprintf(stderr, "Hello: %s\n", (char *) _aget(info, "hello"));
  //fprintf(stderr, "Non-Existing Key: %s\n", (char *) _aget(info, "non_existing_key"));
  if (_ahaskey(info, "non_existing_key")) {
    fprintf(stderr, "Non-Existing Key exists: %s\n", (char *) _aget(info, "non_existing_key"));
  } else {
    fprintf(stderr, "Non-Existing Key does not exist\n");
  }

  DO_FREE(key);
  array_free(info);
  return 0;
}

#define CPU_CORE_STR "cpu_core"
#define OPEN_MAX_STR "open_max"


//#include <mcheck.h>

int main(int argc, char **argv)
{
  //mtrace();

  array_t *cfg = ayubd_config_read("ayubd-ng.conf");

  assert(cfg != NULL);
  system_t *sys = ayubd_init(cfg);
  assert(sys != NULL);
  assert(ayubd_module_load(sys, "http") == 0);
  //signal(SIGHUP, signal_func);

  return ayubd_run(sys);
}

array_t *ayubd_config_read(const char *cfile)
{
  //svc_core_detect_system(); /* dummy function, for experimenting with various things */

  long cval = 0;
  array_t *cfg = array_new();
  if (cfg) {
    cval = sysconf(_SC_NPROCESSORS_ONLN);
    if (cval < 0) cval = 2;
    array_set(cfg, "cpu_core", sizeof("cpu_core"), cval);
    cval = sysconf(_SC_OPEN_MAX);
    if (cval <= 0) cval = 1024;
    array_set(cfg, "open_max", sizeof("open_max"), cval);
  }
  return cfg;
}

int ayubd_module_load(system_t *sys, char *mod_name)
{
#if 0
  module_start(sys);
#endif
  char *error;
  int (*mod_start)(system_t *);
  char sopath[256+1];

  module_t *mod = module_new();
  strncpy(mod->name, mod_name, sizeof(mod->name));
  snprintf(sopath, sizeof(sopath)-1, "modules/%s/mod_%s.so", mod->name, mod_name);
  mod->handle = dlopen(sopath, RTLD_LAZY);
  if (!mod->handle) {
    fprintf(stderr, "Module load failure, %s.\n", dlerror());
    return -1;
  }

  dlerror(); /* clear any existing error */

  mod_start = dlsym(mod->handle, "module_start");
  if ((error = dlerror()) != NULL) {
    fprintf(stderr, "dlsym returned error for function module_start. [%s]\n", error);
    dlclose(mod->handle);
  }

  mod->fd = (*mod_start)(sys); /* invoke the module start function */

  /*
  mod_http_init(sys);
  if (mod_name == NULL) {
  } else {
    if (mod_name[0] != '\0') {
      mod_http_init(sys);
    }
  }
  */

  return 0;
}





system_t *ayubd_init(array_t *cfg)
{
  register int i;
  #if 0
  struct winfo *wi;
  #endif
  hook_t *hp = NULL;
  pthread_t tid;
  pthread_attr_t attr;

#ifdef USE_HS
  hs_compile_error_t *hs_compile_error;
#endif




  system_t *sys = DO_CALLOC(1, sizeof(system_t));
  assert(sys != NULL);
  sys->eventfd = EVQ_CREATE(); /* create event queue (kqueue/epoll) */
  sys->nworkers = array_get(cfg, CPU_CORE_STR, sizeof(CPU_CORE_STR));
  sys->nevents = array_get(cfg, OPEN_MAX_STR, sizeof(OPEN_MAX_STR));
  //sys->q = (struct lfq **) DO_CALLOC(sys->nworkers, sizeof(struct lfq));
  sys->q = DO_CALLOC(sys->nworkers, sizeof(*sys->q));
  sys->qdiv = DO_CALLOC(sys->nworkers, sizeof(*sys->qdiv));

  sys->nsymbols = SYMTBLSZ;
  sys->symtbl = DO_CALLOC(sys->nsymbols, sizeof(*sys->symtbl));
  sys->symtbl[NR_SYS_NWORKERS] = sys->nworkers;
  sys->symtbl[NR_SYS_NEVENTS] = sys->nevents;
  sys->symtbl[NR_SYS_SYSPTR] = (unsigned long int) sys;
  //sys->symtbl = array_new();
  //_aset(sys->symtbl, "sys.nworkers", sys->nworkers);
  //_aset(sys->symtbl, "sys.nevents", sys->nevents);


#ifdef USE_HS
  //_aset(sys->symtbl, "http.route", strdup("/app/(get|verify)\\??"));
  _aset(sys->symtbl, "http.route", strdup("\\{\\s*([a-zA-Z_][a-zA-Z0-9_-]*)\\s*(?::(?:[^{}]*|[^{}]*\\{[^{}]*\\})*\\s*)?\\}")); // FastRoute Syntax
  //_aset(sys->symtbl, "http.route", strdup("(\\?|\\&|;)([^=]+)=([^&|;]+)"));
#endif


  //sys->fq = (struct lfq **) DO_CALLOC(sys->nworkers, sizeof(struct lfq));
  sys->sem = DO_CALLOC(sys->nworkers, sizeof(*sys->sem));

#ifdef USE_LINUX_AIO
  sys->ioq = DO_CALLOC(sys->nworkers, sizeof(*sys->ioq));
  sys->ioqdiv = DO_CALLOC(sys->nworkers, sizeof(*sys->ioqdiv));
#endif


#ifdef USE_CORO
  sys->main_co = DO_CALLOC(sys->nworkers, sizeof(*sys->main_co));
  sys->main_co_sstk = DO_CALLOC(sys->nworkers, sizeof(*sys->main_co_sstk));
#endif

#ifdef USE_REDIS
  sys->redis_ctx = DO_CALLOC(sys->nworkers, sizeof(*sys->redis_ctx));
  sys->redis_reply = DO_CALLOC(sys->nworkers, sizeof(*sys->redis_reply));
#endif

#if 0
  wi = (struct winfo *) DO_CALLOC(sys->nworkers, sizeof(struct winfo));
#endif
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

#ifdef USE_RAND
  long int *srandval = DO_CALLOC(sys->nworkers, sizeof(*srandval));
  ssize_t nr = getrandom(srandval, sizeof(*srandval)*sys->nworkers, 0);
  if (nr < 0) {
    fprintf(stderr, "Error initializing random number!!!\n");
  }
  sys->rand_state = DO_CALLOC(sys->nworkers, sizeof(*sys->rand_state));
  sys->rand_statebuf = DO_CALLOC(sys->nworkers, sizeof(*sys->rand_statebuf));
#endif

#ifdef USE_HS
  sys->hs_database = DO_CALLOC(sys->nworkers, sizeof(*sys->hs_database));
  sys->hs_scratch = DO_CALLOC(sys->nworkers, sizeof(*sys->hs_scratch));
#endif

//  sys->hash = atomic_hash_create(1024*1024, 0);


  /* initialize Asynchronous I/O subsystem */
#ifdef USE_LINUX_AIO
  IO_PREP_INIT(sys->nevents, &sys->ioctx, sys->iocb);
#endif

  for (i = 0; i < sys->nworkers; ++i) {
    hook_new(hp); /* macro, will set hp */
    assert(hp != NULL);
    hp->data = array_new();
    hp->udata[UD_CORE_SYSPTR] = (unsigned long int) sys;
    //_aset(hp->data, "sys", sys); /* hack */
    hp->udata[UD_CORE_NUM] = i;
    //_aset(hp->data, "num", i); /* hack */
    #if 0
    wi[i].sys = sys;
    wi[i].workernum = i;
    #endif

    //sys->q[i] = lfq_new();
    //stailq_new(sys->q[i]);
    stailq_new_lf_spsc(sys->q[i], sys->qdiv[i]);
//    sys->qdiv[i] = NULL;
    //sys->fq[i] = lfq_new();
    sem_init(&sys->sem[i], 0, 0);

#ifdef USE_RAND
    sys->rand_statebuf[i] = DO_CALLOC(1, PRNGBUFSZ);
    initstate_r(srandval[i], sys->rand_statebuf[i], PRNGBUFSZ, &sys->rand_state[i]);
#endif

#ifdef USE_LINUX_AIO
    stailq_new_lf_spsc(sys->ioq[i], sys->ioqdiv[i]);
#endif

#ifdef USE_REDIS
    sys->redis_ctx[i] = redisConnectUnix(REDIS_SOCK);
    if (sys->redis_ctx[i] == NULL || sys->redis_ctx[i]->err) {
      fprintf(stderr, "Error connecting to Redis Socket [i=%d]\n", i);
    }
    sys->redis_reply[i] = redisCommand(sys->redis_ctx[i], "PING");
    fprintf(stderr, "PING[%d]: %s\n", i, sys->redis_reply[i]->str);
    freeReplyObject(sys->redis_reply[i]);
#endif

#ifdef USE_HS
    if (hs_compile((char *) _aget(sys->symtbl, "http.route"),
      HS_FLAG_CASELESS | HS_FLAG_UTF8 | HS_FLAG_UCP | HS_FLAG_SOM_LEFTMOST, /* http://intel.github.io/hyperscan/dev-reference/api_files.html#c.hs_compile */
      HS_MODE_BLOCK, NULL, &sys->hs_database[i], &hs_compile_error) != HS_SUCCESS) {
      fprintf(stderr, "Error compiling regex (http.route)!!!\n");
      hs_free_compile_error(hs_compile_error);
      exit(EXIT_FAILURE); // fatal error
    }

    if (hs_alloc_scratch(sys->hs_database[i], &sys->hs_scratch[i]) != HS_SUCCESS) {
      hs_free_database(sys->hs_database[i]);
      exit(EXIT_FAILURE); // fatal error
    }
#endif

    pthread_create(&tid, NULL, ayubd_worker, (void *) hp);
  }

#ifdef USE_RAND
  DO_FREE(srandval);
#endif

  pthread_attr_destroy(&attr);

  fprintf(stderr, "Going to initialize system with %d workers and %d events\n",
      sys->nworkers, sys->nevents);

  return sys;
}

int ayubd_run(system_t *sys)
{
  int nfds;
  register int n;
  int x;
  sys->events = (event_t *) DO_CALLOC(sys->nevents, sizeof(event_t));
  if (sys->events == NULL) {
    err_exit("calloc: MAXEVENTS * event_t");
  }
  event_t ev;
  hook_t *hp;
  //const int conndiffmax = 500;

  fprintf(stderr, "Running event loop\n");


  while (1) {
    /* adjust the event list size as per active connections */
    #if 0
    if (sys->nconns > conndiffmax && abs(sys->nconns - sys->nevents) > conndiffmax) {
      fprintf(stderr, "Changing events array size. Old: %d, New: %d\n", sys->nevents, sys->nconns);
      sys->nevents = sys->nconns;
      sys->events = DO_REALLOC(sys->events, sizeof(event_t) * sys->nevents);
      if (sys->events == NULL) {
        fprintf(stderr, "Mememory allocation failure: %s:%d:%s\n", __FILE__, __LINE__, __func__);
	exit(EXIT_FAILURE);
      }
    }
    #endif
    nfds = EVQ_POLL(sys->eventfd, sys->events, sys->nevents);
    if (nfds == -1) {
      err_warn("EVQ_POLL");
      continue;
    }
    //if (nfds > 999) fprintf(stderr, "nfds=%d\n", nfds);
    /*
    if (sys->nconns_max < nfds) sys->nconns_max = nfds;
    */

    for (n = 0; n < nfds; ++n) {
      ev = sys->events[n];
      hp = ev.data.ptr;
      hp->ev.events = ev.events; /* update the event flags, also it ensures we initialized things before we read data */
      /* invoke hook functions in here */
      if (hp && hp->ready) { /* ready is needed to detect abnormally closed sockets */
        if (hp->ready(hp) > 0 && hp->process) { /* if returns non-NULL, and process is not-NULL */
	  x = hp->fd % sys->nworkers; /* in case no fd is associated, thread 0 will process it */
	  hook_ref(hp); /* make sure main thread/worker thread doesn't free it accidentally */
	  //lfq_in(sys->q[x], hp);
	  //stailq_in(sys->q[x], hp);
	  stailq_in_lf_spsc(sys->q[x], sys->qdiv[x], hp);
	  sem_post(&sys->sem[x]);
	}
      }
    }
  }
  return 0;
}

void ayubd_quit(system_t *sys)
{
  if (sys) {
    if (sys->q) DO_FREE(sys->q);
    DO_FREE(sys);
  }
}

/*
 * runs on a pthread
 * need to use coroutine to process the request
 */

#ifdef USE_CORO
void process_func(void)
#else
void process_func(void *s)
#endif
{
  hook_t *hp =
#ifdef USE_CORO
  aco_get_arg()
#else
  s
#endif
  ;
  /* we are looping so that we can reuse same coroutine to process more than once (reusing session) */
#ifdef USE_CORO
  while (hp->process) {
    hp->process(hp);
    aco_yield();
  }
  aco_exit();
#else
  if (hp->process) hp->process(hp);
#endif
}

void *ayubd_worker(void *arg)
{
  system_t *sys;
  int x;
#if 0
  int xx, ii;
  int sleeptime = SLEEPTIME;
#endif

#if 0
  hook_arg_t *harg = DO_CALLOC(1, sizeof(hook_arg_t));
  struct winfo *wi = (struct winfo *) arg;
#endif

  hook_t *s;

  hook_t *hp = (hook_t *) arg;
  //sys = (system_t *) _aget(hp->data, "sys");
  sys = (system_t *) hp->udata[UD_CORE_SYSPTR];
  //x = (int) _aget(hp->data, "num");
  x = (int) hp->udata[UD_CORE_NUM];
  assert(sys != NULL);
  #if 0
  sys = wi->sys;
  x = wi->workernum;
  #endif

#ifdef USE_CORO
  aco_thread_init(NULL);

  sys->main_co[x] = aco_create(NULL, NULL, 0, NULL, NULL);
  sys->main_co_sstk[x] = aco_share_stack_new(0);
#endif

  while (1) {
    sem_wait(&sys->sem[x]);
    //s = lfq_out(sys->q[x]); /* dequeue if any, from own queue */
    //stailq_out(sys->q[x], s); /* dequeue if any, from own queue */
    stailq_out_lf_spsc(sys->q[x], sys->qdiv[x], s); /* dequeue if any, from own queue */
    if (s == NULL) { /* nothing much yet */
      fprintf(stderr, "Q Empty, even though we were alerted! Need checking on Queue Code.\n");
      /*
      USLEEP(sleeptime);
      if (sleeptime < MAXSLEEPTIME) sleeptime += SLEEPTIME;
      */
      /* no events in queue */
      continue;
    }
#if 0
    else {
      sleeptime = SLEEPTIME; /* reset sleeptime */
    }
#endif

    /* now process the event */
    if (s->process) {
      s->udata[UD_CORE_NUM] = x;
      //_aset(s->data, "num", x); /* we may batch I/O to the queue */
      s->udata[UD_CORE_SYSPTR] = (unsigned long int) sys;
#ifdef USE_CORO
      hook_exec(s, x, process_func, s); /* make sure coroutine is created (if not already done) */
#else
      s->process(s);
#endif
      hook_unref(s); /* before putting into queue, main thread does 'hook_ref()' so that it doesn't free accidently while worker is processing it */
    }
    /*
    http_process_data(s);
    */
  }

#ifdef USE_CORO
  aco_share_stack_destroy(sys->main_co_sstk[x]);
  aco_destroy(sys->main_co[x]);
#endif

  fprintf(stderr, "Should Never Reach Here: %s:%d:%s\n", __FILE__, __LINE__, __func__);

  hook_free(hp);
  pthread_exit(NULL);
  return NULL;
}

