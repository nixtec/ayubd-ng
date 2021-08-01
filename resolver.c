/*
 * resolver.c
 */
#include <netdb.h>
#include <signal.h>
#include <sys/signalfd.h>

int resolver_init(void)
{
  int sfd;
  sigset_t mask;

  sigemptyset(&mask);
  sigaddset(&mask, SIGRTMIN);
  sigprocmask(SIG_BLOCK, &mask, NULL);
  sfd = signalfd(-1, 
}
