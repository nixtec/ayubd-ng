/*
 * ioqueue_signal.c
 */
#include "ioqueue_signal.h"


/*
 * The use of sigprocmask() is unspecified in a multithreaded process;
 * see pthread_sigmask(3).
 */
ioqueue_signal_t *ioqueue_signal_new(int flags)
{
  ioqueue_signal_t *iosig = DO_CALLOC(1, sizeof(ioqueue_signal_t));
  if (iosig) {
    iosig->flags = flags;
  }
  return iosig;
}

int ioqueue_signal_init(ioqueue_signal_t *iosig)
{
  sigset_t mask;
  sigfillset(&mask);
  assert(pthread_sigmask(SIG_BLOCK, &mask, NULL) == 0);
  int sfd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
  if (sfd == -1) return -1;

  ptr = DO_REALLOC(iosig->fd, iosig->fdlen+1);
  if (ptr) {
    iosig->fd = ptr;
    iosig->fd[iosig->fdlen]
  }


}
