/*
 * iomsg.h
 */
#ifndef __IOMSG_H__
#define __IOMSG_H__

typedef struct msghdr iomsg_t;
iomsg_t *iomsg_new(void);
int iomsg_set_addr(iomsg_t *msg, void *addr, socklen_t addrlen);


#endif
