/*
 * ioqueue_socket.c
 */
#include "ioqueue_socket.h"


//static _Atomic int gcnt;

#if 0
void msg_io_push(void *hdata, struct msghdr *imsg, const char *buf, size_t buflen)
{
  hook_t *hp = hdata;

  imsg->msg_iovlen++;
  imsg->msg_iov = DO_REALLOC(imsg->msg_iov, imsg->msg_iovlen * sizeof(*imsg->msg_iov));
  assert(imsg->msg_iov != NULL);
  imsg->msg_iov[msg_iovlen-1] = DO_MALLOC();
}
#endif

ssize_t ioqueue_socket_recvmsg(void *hdata, int flags)
{
  int i;
  ssize_t n;
  struct msghdr *imsg;

  hook_t *hp = hdata;
  assert(hp != NULL);
  assert(hp->data != NULL);

  if (!(imsg = (typeof (imsg)) hp->udata[UD_CORE_MSGHDR])) {
    hook_zalloc(hp, imsg, sizeof(*imsg)); /* push to memory pool stack */
    /* if msghdr is not set, it means the socket is not connected (maybe datagram socket) (otherwise _on_connect hook would set it) */
    /* so we are going to allocate address storage */
    imsg->msg_namelen = sizeof(struct sockaddr_storage);
    hook_zalloc(hp, imsg->msg_name, imsg->msg_namelen);
    //_aset(hp->data, "msghdr", imsg); /* setting up existing key may make memory leak (unless it's allocated using hook_*alloc() */
    hp->udata[UD_CORE_MSGHDR] = (unsigned long int) imsg;
  }
  assert(imsg != NULL);

  if (!imsg->msg_iov || imsg->msg_iovlen == 0) {
    imsg->msg_iovlen = 2; /* for sending back http header+body, having two buffer is convenient */
    hook_zalloc(hp, imsg->msg_iov, imsg->msg_iovlen * sizeof(*imsg->msg_iov));
    //imsg->msg_iov = DO_MALLOC(imsg->msg_iovlen * sizeof(*imsg->msg_iov));
    assert(imsg->msg_iov != NULL);

    for (i = 0; i < imsg->msg_iovlen; ++i) {
      hook_zalloc_iov(hp, &imsg->msg_iov[i]);
    }
  } else {
    for (i = 0; i < imsg->msg_iovlen; ++i) {
      IOV_RESET(&imsg->msg_iov[i]); /* iov_len fields are updated upon response to clients */
    }
  }

  DO_IOCALL(n,recvmsg(hp->fd, imsg, 0)); /* this will ensure the EINTR or similar things are handled */
  /*
  assert(write(STDERR_FILENO, imsg->msg_iov[0].iov_base, MIN(n,IOV_CHUNK)) == MIN(n,IOV_CHUNK));
  assert(write(STDERR_FILENO, "\n", 1) == 1);
  */

  return n;
}

ssize_t ioqueue_socket_sendmsg(void *hdata, int flags)
{
  ssize_t n;
  hook_t *hp = hdata;
  //struct msghdr *imsg = (typeof(imsg)) _aget(hp->data, "msghdr");
  struct msghdr *imsg = (typeof (imsg)) hp->udata[UD_CORE_MSGHDR];
  assert(imsg != NULL);

  //fprintf(stderr, "num=%d\n", (int) _aget(hp->data, "num"));

  DO_IOCALL(n, sendmsg(hp->fd, imsg, 0));

  return n;
}


/* to be executed by worker thread
 * so hp must be valid and hp->data must also be valid
 */
int ioqueue_socket_process_data(void *arg)
{
  int ret = 0;
  hook_t *hp = arg;
  hook_func_t hfunc;

  assert(hp != NULL);
  assert(hp->data != NULL);

  /* hook data function is mandatory, otherwise just close socket */
  if ((hfunc = (hook_func_t) _aget(hp->data, "hoda")) != NULL) {
    ret = hfunc(hp);
    if (ret <= 0) {
      goto cleanup;
    }
    return ret;
  }

cleanup:
  //hook_close(hp); /* this will unregister socket from event queue, so no more events might be generated */
  return ret;
}


/* this should be executed by 'main' thread, so it should be in 'ready' hook func */
int ioqueue_socket_on_data(void *arg)
{
  int ret = 0;
  hook_t *hp = (hook_t *) arg;
  assert(hp != NULL);
  assert(hp->data != NULL);
#ifdef USE_HOCL
  hook_func_t hfunc;
#endif

  if (CHECKFLAG(hp->ev.events, EPOLLRDHUP) ||
      CHECKFLAG(hp->ev.events, EPOLLHUP) ||
      CHECKFLAG(hp->ev.events, EPOLLERR)) {

#ifdef USE_HOCL
    /* need to know if this is applicable for datagram sockets */
    if (_ahaskey(hp->data, "hocl")) { /* has hook_on_close installed */
      hfunc = (hook_func_t) _aget(hp->data, "hocl"); /* this function must not block and should be fast */
      /* later we will implement efficient Message Queue to execute such functions */
      if (hfunc) hfunc(hp); /* run in main thread context */
    }
#endif
  } else {

    if (!hp->process) {
      if (_ahaskey(hp->data, "hoda")) {
	hp->process = ioqueue_socket_process_data; /* default handler for socket when data is available for reading */
	ret = 1;
      }
    } else {
      ret = 1;
    }
  }

  if (ret == 0) {
    hook_close(hp); /* this will unregister socket from event queue, so no more events might be generated */
    //--gcnt; fprintf(stderr, "gcnt = %d\n", gcnt);
  }

  return ret;
}



/* what to do when someone is connected */
/* this function is executing in main thread, so our constraint on single producer and single consumer won't violate */
int ioqueue_socket_on_connect(void *arg)
{
  hook_t *hp = (hook_t *) arg;
  assert(hp != NULL);
  assert(hp->data != NULL);
#ifdef USE_HOCO
  hook_func_t hfunc;
#endif

  int connfd;
  struct sockaddr_storage remote_addr;
  struct msghdr *imsg = NULL;
  socklen_t addrlen = sizeof(remote_addr);
  //system_t *sys = (system_t *) _aget(hp->data, "sys"); /* find a way to optimise, we don't want a lookup (even though fast) on every connect returns */
  system_t *sys = (system_t *) hp->udata[UD_CORE_SYSPTR];
  assert(sys != NULL);
  hook_t *hs; /* hook session */
  while ((connfd = ACCEPT(hp->fd, (struct sockaddr *) &remote_addr, &addrlen)) > 0 || errno == EINTR) {
    if (connfd == -1) continue; /* retry, in case of EINTR */

    hook_new(hs);
    if (hs) {
      hook_zalloc(hs, imsg, sizeof(*imsg)); /* push to memory pool stack */
      hs->fd = connfd;
      hs->ready = ioqueue_socket_on_data; /* this function detects errors/closes on socket, in main thread */
      array_dup(hp->data, hs->data); /* carry on the user (module) supplied data [macro, so hs->data will be updated] */
      UDATA_DUP(hp, hs); /* copy ->udata elements from hp */
      imsg->msg_namelen = sizeof(struct sockaddr_storage);
      hook_zalloc(hs, imsg->msg_name, imsg->msg_namelen);

      memcpy(imsg->msg_name, &remote_addr, addrlen);
      //_aset(hs->data, "msghdr", imsg); /* setting up existing key may make memory leak (unless it's allocated using hook_*alloc() */
      hs->udata[UD_CORE_MSGHDR] = (unsigned long int) imsg;
      //hs->udata[UD_CORE_SYSPTR] = hp->udata[UD_CORE_SYSPTR];
      //hs->datalen = hp->datalen; /* carry on the user (module) supplied data */
      hs->ev.events = EPOLLIN | EPOLLRDHUP | EPOLLPRI | EPOLLET;
      hs->ev.data.ptr = hs;


#ifdef USE_HOCO
      if (_ahaskey(hs->data, "hoco")) { /* has hook_on_connect installed */
	hfunc = (hook_func_t) _aget(hs->data, "hoco"); /* this function must not block, must be fast enough as well. otherwise main thread will delay to respond. */
	if (hfunc) ret = hfunc(hs); /* run in main thread context */
	if (ret <= 0) {
	  hook_close(hs);
	  continue;
	}
      }
#endif
      //setsockopt(connfd, IPPROTO_IP, TCP_NODELAY, &value, sizeof(value));
      EVQ_ADD(sys->eventfd, hs->fd, &hs->ev);
      //++gcnt;
    } else {
      close(connfd);
    }
  }

  return 0; /* this is server socket, it will never receive data, unless it's a datagram socket, in which case *_on_connect is meaningless */
}


/* node may be 'NULL' to bind to wildcard address */
/* const char *node, const char *serv */
int ioqueue_socket_prepare(ioqueue_t *iq, array_t *args)
{
  assert(iq != NULL);
  assert(args != NULL);

  const char *node = (const char *) _aget(args, "node");
  const char *serv = (const char *) _aget(args, "serv");

  struct sockaddr_storage addr;
  struct sockaddr_un *unix_addr;
#if 0
  mode_t mask, omask;
#endif

  struct addrinfo hints, *result, *rp;
  int errcode;
  int sockfd;
  void *ptr;
  int one;

  memset(&hints, 0, sizeof(hints));
  /* currently only TCP and UDP are handled, later may add more support */
  if (iq->flags & IOQUEUE_SOCKET_UDP) {
    hints.ai_socktype = SOCK_DGRAM;
  } else {
    hints.ai_socktype = SOCK_STREAM;
    ioqueue_set_flags(iq, IOQUEUE_SOCKET_TCP);
    //iq->flags |= IOQUEUE_SOCKET_TCP;
  }

  if (iq->flags & IOQUEUE_SOCKET_IP4) {
    hints.ai_family = AF_INET;
  } else if (iq->flags & IOQUEUE_SOCKET_IP6) {
    hints.ai_family = AF_INET6;
  } else if (iq->flags & IOQUEUE_SOCKET_UNIX) {
    hints.ai_family = AF_UNIX;
    /* remove previously bound socket */
    if (access(node, F_OK) != -1 && unlink(node) == -1) {
      perror("unlink");
      return -1;
    }
  } else { /* default */
    hints.ai_family = AF_UNSPEC;
    ioqueue_set_flags(iq, IOQUEUE_SOCKET_IP46);
    //iq->flags |= IOQUEUE_SOCKET_IP46;
  }

  if (!(iq->flags & IOQUEUE_SOCKET_ACTIVE)) {
    hints.ai_flags |= AI_PASSIVE;
    ioqueue_set_flags(iq, IOQUEUE_SOCKET_PASSIVE);
    //iq->flags |= IOQUEUE_SOCKET_PASSIVE;
  }

  if (iq->flags & IOQUEUE_SOCKET_UNIX) {
    memset(&addr, 0, sizeof(addr));
    addr.ss_family = hints.ai_family;
    unix_addr = (typeof(unix_addr)) &addr;
    unix_addr->sun_family = hints.ai_family;
    strncpy(unix_addr->sun_path, node, sizeof(unix_addr->sun_path)-1); /* make sure the length is correct */
    hints.ai_addr = (typeof(hints.ai_addr)) unix_addr;
    hints.ai_addrlen = sizeof(*unix_addr);
    //hints.ai_addrlen = sizeof(addr);

    //fprintf(stderr, "Unix Domain Socket: %s\n", unix_addr->sun_path);

    result = &hints;

  } else {

    #if 0
    hints.ai_flags |= AI_CANONNAME; /* fill up ai_canonname with string hostname (official name) */
    #endif
    if (seems_ip4(node) || seems_ip6(node)) { /* this may save a DNS lookup (in case dotted IP is provided) */
      //fprintf(stderr, "Numeric Host: %s\n", node);
      hints.ai_flags |= AI_NUMERICHOST;
    }
    if (ctype_digit(serv)) { /* this may save a /etc/services file lookup (in case numeric port is provided) */
      //fprintf(stderr, "Numeric Serv: %s\n", serv);
      hints.ai_flags |= AI_NUMERICSERV;
    }

    errcode = getaddrinfo(node, serv, &hints, &result);
    if (errcode != 0) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(errcode));
      return -1;
    }
  }

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    #if 0
    if (rp == result) { /* canonical name is only available in first result */
      fprintf(stderr, "Canonical Name: %s\n", rp->ai_canonname);
    }
    #endif
    sockfd = socket(rp->ai_family, rp->ai_socktype | SOCK_NONBLOCK | SOCK_CLOEXEC, rp->ai_protocol);
    if (sockfd == -1) {
      perror("socket");
      continue;
    }

    if (iq->flags & IOQUEUE_SOCKET_PASSIVE) {
      /* for server socket, we might need to be able to reuse address (that is safe to listen to) */
      one = 1;
      setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

      /* https://stackoverflow.com/questions/11781134/change-linux-socket-file-permissions */
      /* On Linux, you need to call fchmod() on the Unix domain socket file descriptor before bind(). In this way the bind() call will create the filesystem object with the specified permissions. Calling fchmod() on an already bound socket is not effective.
       *
       * Using chmod() could lead to TOCTTOU race condition. If possible, use fchmod() instead.
       */
      //fprintf(stderr, "ai_addr: %s\n", ((struct sockaddr_un *) rp->ai_addr)->sun_path);
#if 0
      /* FIXME: AYUB: But, the fact is, the socket is not yet created, so we can't call fchmod() effectively here */
      /* unix(7) sockets are created using umask(2) values, so we're switching between umask values */
      mask = 0111; /* we want 0666 to be set, so using 0111 */
      omask = umask(mask);
#endif
      if (bind(sockfd, rp->ai_addr, rp->ai_addrlen)  != 0) {
	perror("bind");
	close(sockfd);
#if 0
	umask(omask);
#endif
	continue;
      }
#if 0
      umask(omask); /* restore */
#endif

      /* chmod(2) might be effective, but fchmod() is not working. so we're using umask() above, so that upon bind it sets permission properly */
      if (rp->ai_family == AF_UNIX || rp->ai_family == AF_LOCAL) {
        /* fchmod(3) said that for socket the behavior is unspecified */
	fprintf(stderr, "ai_addr: %s\n", ((struct sockaddr_un *) rp->ai_addr)->sun_path);
	if (chmod(((struct sockaddr_un *) rp->ai_addr)->sun_path, 0666) != 0) {
	  perror("chmod");
	}
      }
      
    } else {
      /* active socket connections are not yet handled, needs more planning */
    }
    ptr = DO_REALLOC(iq->fd, iq->fdlen+1);
    if (ptr) {
      iq->fd = ptr;
      iq->fd[iq->fdlen] = sockfd;
      iq->fdlen++;
    } else { /* memory allocation failed, close socket */
      close(sockfd);
      continue;
    }
    //fprintf(stderr, "Bound socket successfully!\n");
  }
  if (result != &hints) freeaddrinfo(result);

  if (iq->fdlen == 0) {
    fprintf(stderr, "Could not bind to any socket. Please check and try again.\n");
  }

  return 0;
}


int ioqueue_socket_deploy(ioqueue_t *iq, array_t *args)
{
  assert(iq != NULL);
  assert(args != NULL);

  hook_func_t ready_hook_func = ioqueue_socket_on_connect; /* default */

  int i;
  int sockfd;
  hook_t *hp;

  system_t *sys = (system_t *) _aget(args, "sys");
  assert(sys != NULL);

  for (i = 0; i < iq->fdlen; i++) {
    sockfd = iq->fd[i];

    if (iq->flags & IOQUEUE_SOCKET_PASSIVE) {
      if (listen(sockfd, 8192) == -1) {
	perror("listen");
	return -1;
      }

      if (iq->flags & IOQUEUE_SOCKET_TCP) {
	ready_hook_func = ioqueue_socket_on_connect; /* passive connect */
      } else { /* datagram, directly respond to data */
	ready_hook_func = ioqueue_socket_on_data;
      }

    } else {
      /* try to connect, nonblockingly */
      // hook_func = ioqueue_socket_on_connect_active;
    }

    hook_new(hp);
    if (hp) {
      hp->fd = sockfd;
      hp->ready = ready_hook_func;
      hp->data = args;
      hp->udata[UD_CORE_SYSPTR] = (unsigned long int) sys;
      if (_ahaskey(args, "route")) {
	hp->udata[UD_MOD_HTTP_ROUTE] = (unsigned long int) _aget(args, "route");
      }
      //hp->datalen = array_size(args); /* just in case needed */
      if (iq->flags & IOQUEUE_SOCKET_PASSIVE) {
	hp->ev.events = EPOLLIN;
      } else {
	hp->ev.events = EPOLLIN | EPOLLOUT;
      }
      hp->ev.data.ptr = hp;
      EVQ_ADD(sys->eventfd, sockfd, &hp->ev); /* add to main event queue */
    } else {
      fprintf(stderr, "Unable to create hook: %s:%d\n", __FILE__, __LINE__);
      return -1;
    }
  }

  return 0;
}


