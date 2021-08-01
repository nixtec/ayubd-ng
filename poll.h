/*
 * poll.h
 * (e)poll/kqueue wrapper according to system being used
 */

#ifdef __linux__
#define USE_EPOLL
#else
#define USE_KQUEUE
#endif

#ifdef USE_EPOLL
#include <sys/epoll.h>

typedef struct epoll_event event_t;
#define EVQ_CREATE() epoll_create1(EPOLL_CLOEXEC)
#define EVQ_CTL(evq,fd,cmd,evp) do { \
  epoll_ctl((evq),(cmd),(fd),(evp)); \
} while (0)
#define EVQ_ADD(evq,fd,evp) EVQ_CTL(evq,fd,EPOLL_CTL_ADD,evp)
#define EVQ_DEL(evq,fd) EVQ_CTL(evq,fd,EPOLL_CTL_DEL,NULL)
#define EVQ_POLL(evq,evlist,nevents) epoll_wait((evq), (evlist),(nevents), -1)
#define EV_GETFD(ev) ((ev).data.fd)
#define EV_GETPTR(ev) ((ev).data.ptr)
#define EVP_GETFD(evp) ((evp)->data.fd)
#define EVP_GETPTR(evp) ((evp)->data.ptr)

#ifdef USE_LINUX_AIO
#include <linux/aio_abi.h>
#include <sys/syscall.h>
#define IO_SETUP(n,pctx) syscall(__NR_io_setup, (n), (pctx))
#define IO_DESTROY(ctx) syscall(__NR_io_destroy, (ctx))
#define IO_SUBMIT(ctx,n,ppiocb) syscall(__NR_io_setup, (ctx), (n), (ppiocb))
#define IO_GETEVENTS(ctx,minn,maxn,pevents,ptimeout) syscall(__NR_io_getevents, (ctx), (minn), (maxn), (pevents), (ptimeout))

#define IO_PREP_RD IOCB_CMD_PREADV
#define IO_PREP_WR IOCB_CMD_PWRITEV
#define IO_PREP_VECTOR(op,piocb,fd,piov,iovlen,offset) 				\
do {										\
  memset((piocb), 0, sizeof(*(piocb)));						\
  (piocb)->aio_fildes = (fd);							\
  (piocb)->aio_lio_opcode = (op);						\
  (piocb)->aio_buf = (typeof((piocb)->aio_buf)) (piov);				\
  (piocb)->aio_nbytes = (iovlen);						\
  (piocb)->aio_offset = (offset);						\
} while (0)
#define IO_PREP_RDV(piocb,fd,piov,iovlen,offset) IO_PREP_VECTOR((IO_PREP_RD),(piocb),(fd),(piov),(iovlen),(offset))
#define IO_PREP_WRV(piocb,fd,piov,iovlen,offset) IO_PREP_VECTOR((IO_PREP_WR),(piocb),(fd),(piov),(iovlen),(offset))

#define IO_PREP_INIT(n,pctx,ppiocb) 							\
do {										\
  *(pctx) = 0;									\
  assert(IO_SETUP((n), (pctx)) == 0);						\
  (ppiocb) = DO_CALLOC((n), sizeof(*(ppiocb)));					\
  int ii;									\
  for (ii = 0; ii < (n); ++ii) {						\
    (ppiocb)[ii] = DO_CALLOC(1, sizeof(**(ppiocb)));				\
  }										\
} while (0)
#endif



#endif

#ifdef USE_KQUEUE
/* MACOSX */
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

typedef struct kevent event_t;
#define EVQ_CREATE() kqueue()
/* evp = &ev */
#define EVQ_CTL(evq,fd,cmd,evp) do { \
  EV_SET((evp), (fd), EVFILT_READ, (cmd), 0, 0, 0); \
  kevent((evq), (evp), 1, (const event_t *) 0, 0, (struct timespec *) 0); \
} while (0)
#define EVQ_ADD(evq,fd,evp) EVQ_CTL(evq,fd,EV_ADD,evp)
#define EVQ_DEL(evq,fd,evp) EVQ_CTL(evq,fd,EV_DELETE,evp)
#define EVQ_POLL(evq,evlist,nevents) kevent((evq), 0, 0, (evlist),(nevents), (const struct timespec *) 0)
#define EV_GETFD(ev) ((ev).ident)
#define EV_GETPTR(ev) ((ev).udata)
#define EVP_GETFD(evp) ((evp)->ident)
#define EVP_GETPTR(evp) ((evp)->udata)

#endif

