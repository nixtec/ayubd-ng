/*
 * ioqueue_socket.h
 */
#ifndef __IOQUEUE_SOCKET_H__
#define __IOQUEUE_SOCKET_H__

#include "main.h"
#include "ioqueue.h"

#define IOQUEUE_SOCKET_TCP (1<<0)
#define IOQUEUE_SOCKET_UDP (1<<1)
#define IOQUEUE_SOCKET_IP4 (1<<2)
#define IOQUEUE_SOCKET_IP6 (1<<3)
#define IOQUEUE_SOCKET_IP46 (1<<4)
#define IOQUEUE_SOCKET_UNIX (1<<5)
#define IOQUEUE_SOCKET_ACTIVE (1<<6)
#define IOQUEUE_SOCKET_PASSIVE (1<<7)

int ioqueue_socket_prepare(ioqueue_t *iq, array_t *args);
int ioqueue_socket_deploy(ioqueue_t *iq, array_t *args);
ssize_t ioqueue_socket_recvmsg(void *hdata, int flags);
ssize_t ioqueue_socket_sendmsg(void *hdata, int flags);
#if 0
int ioqueue_socket_on_connect(void *arg);
int ioqueue_socket_on_data(void *arg);
int ioqueue_socket_process_data(void *arg);
#endif

#endif
