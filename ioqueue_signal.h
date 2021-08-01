/*
 * ioqueue_signal.h
 */
#ifndef __IOQUEUE_SIGNAL_H__
#define __IOQUEUE_SIGNAL_H__

#include <signal.h>
#include <sys/signalfd.h>

#include 

typedef struct {
  int flags;
  int *fd; /* usually single file descriptor, but who knows we might think of something crazy later */
  int fdlen;
} ioqueue_signal_t;
