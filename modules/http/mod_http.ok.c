/*
 * mod_http.c
 * http module
 */
#include "mod_http.h"

//#define _DEBUG 1


static void http_fragment_cb(void *data, const char *at, size_t len)
{
#ifdef _DEBUG
  DUMP(at,len);
#endif
  return;
}

static void http_request_uri_cb(void *data, const char *at, size_t len)
{
  /*
   * will be called when request uri is parsed
   */
#ifdef _DEBUG
  DUMP(at,len);
#endif

#if 0
  http_session_data_t *sd = data;
  char buf[80];
  char key[40];
  char val[40];
  int nitems = 0;
#define PREFIX "/result/"
  if (strncmp(at, PREFIX, sizeof(PREFIX)-1) == 0) {
    memset(buf, 0, sizeof(buf));
    memcpy(buf, at, len);
    nitems = sscanf(buf, PREFIX "%u/%u", &sd->board, &sd->roll);
    if (nitems < 2) {
      fprintf(stderr, "Less than two items received, Ignoring!\n");
    } else {
      //fprintf(stderr, "Board: %u, Roll: %u\n", sd->board, sd->roll);
      sprintf(key, "%u/%u", sd->board, sd->roll);
      sprintf(val, "%s: Passed with GPA 5.00", key);
      if (!lru_find_in_cache(key)) lru_add_to_cache(key, val);
    }
  }
  /* this is a design problem if we write code in mod_http.c */
#endif
  return;
}

static void http_request_method_cb(void *data, const char *at, size_t len)
{
#ifdef _DEBUG
  DUMP(at,len);
#endif
  if (HTTP_VERIFY_METHOD(at, len) == ERROR) { fprintf(stderr, "Error in method\n"); }
}

static void http_request_path_cb(void *data, const char *at, size_t len)
{
  /*
   * will be called when request path is parsed
   */
#ifdef _DEBUG
  DUMP(at,len);
#endif
  return;
}


static void http_field_cb(void *data,
    const char *field, size_t flen,
    const char *value, size_t vlen)
{
  /*
  http_session_data_t *sd = data;

  if (hrq_key_id(field, flen) < 0) {
    write(STDERR_FILENO, field, flen);
    write(STDERR_FILENO, "\n", 1);
  }
  */
  /*
   * header fields in key (field), value (value)
   */
#ifdef _DEBUG
  DUMPX(field,flen,value,vlen);
#endif
}

static void http_query_string_cb(void *data, const char *at, size_t len)
{
  /*
   * will be called if any query string passed
   */
#ifdef _DEBUG
  DUMP(at,len);
#endif
}

static void http_version_cb(void *data, const char *at, size_t len)
{
#ifdef _DEBUG
  DUMP(at,len);
#endif
}


static void http_header_done_cb(void *data, const char *at, size_t len)
{
#ifdef _DEBUG
  DUMP(at,len);
#endif
}

static int http_parse_header(http_session_data_t *sd, size_t off)
{
  int retval = -1;
  http_parser parser;
  http_parser_init(&parser);

  /*
  parser.http_field = http_field_cb;
  parser.request_method = http_request_method_cb;
  parser.request_uri = http_request_uri_cb;
  parser.fragment = http_fragment_cb;
  parser.request_path = http_request_path_cb;
  parser.query_string = http_query_string_cb;
  parser.http_version = http_version_cb;
  parser.header_done = http_header_done_cb;
  */

#if 0
  DEBUG(stderr, "===== HEADER START =====\n");
  write(2, header, len);
  write(2, "\n", 1);
  DEBUG(stderr, "=====  HEADER END  =====\n");
#endif

  parser.data = (void *) sd;
  char *header = sd->iov[0].iov_base;
  size_t len = sd->iov0_pos;
  retval = http_parser_execute(&parser, header, len, off);
  if (!http_parser_is_finished(&parser) || http_parser_has_error(&parser)) {
    /*
    DEBUG(stderr, "HTTP Header not complete or with error!\n");
    */
    retval = -1;
  }
#if 0
  else {
    fprintf(stderr, "Header parse finished\n");
  }
  fprintf(stderr, "retval=%d\n", retval);
#endif

  return retval;
}

static void *http_process_data(void *s, void *sys)
{
  ssize_t n = 0;
#ifndef MAXBUFSIZ
#define MAXBUFSIZ 4096
#endif
  char buf[MAXBUFSIZ];
  ssize_t nwritten;
  char *ptr;
  ssize_t err = 0;
  size_t tlen; /* temporary length calculation */

  char *contents = NULL;
  contents = malloc(4096);
  long int rresult = 0;
  system_t *ss = sys;

#if 0
  prog_t *pr = NULL;
  prog_out_t *po = NULL;
#endif

  http_session_t *hs = (http_session_t *) s;
  http_session_data_t *sd = (http_session_data_t *) hs->data;
  if (!sd) {
    /*
    DEBUG(stderr, "Session Data NULL. Skipped processing.!!!\n");
    */
    err = -1;
    goto yield;
  }

  while ((n = recv(hs->fd, buf, sizeof(buf)-1, 0)) > 0) {
#if 0
    fprintf(stderr, "Received data\n");
#endif
    if (sd->iov0_pos + n >= HTTP_MAXHEADER) {
      fprintf(stderr, "Exceeded maximum header size %d.\n", HTTP_MAXHEADER);
      err = -1;
      goto yield;
    }
    /*
    if ((sd->iov0_pos + n) >= sd->iov[0].iov_len) {
      fprintf(stderr, "Expanding space in iov[0], newsize = %d\n", (int) (sd->iov0_pos+n+1));
      ptr = realloc(sd->iov[0].iov_base, sd->iov0_pos + n + 1);
      if (ptr) {
	sd->iov[0].iov_base = ptr;
	sd->iov[0].iov_len = sd->iov0_pos + n + 1;
      } else {
	err = -1;
	goto yield;
      }
    }
    */

    memcpy((sd->iov[0].iov_base)+(sd->iov0_pos), buf, n);
    sd->iov0_pos += n;

#if 0
    temp += n;
    fprintf(stderr, "Going to read more data.\n");
#endif
    /* consume data */
  }

  *(char *) ((char *) sd->iov[0].iov_base+sd->iov0_pos) = '\0';
#if 0
  write(2, sd->iov[0].iov_base, sd->iov0_pos);
  write(2, "\n", 1);
#endif
  if (strstr(sd->iov[0].iov_base, "\r\n\r\n") ||
      strstr(sd->iov[0].iov_base, "\n\n") ||
      strstr(sd->iov[0].iov_base, "\r\r")) {
#if 0
    fprintf(stderr, "strlen iov[0] = %d, pos = %d\n", strlen(sd->iov[0].iov_base), sd->iov0_pos);
    fprintf(stderr, "header dump:\n%s\n", (char *) sd->iov[0].iov_base);
    err = http_parse_header(sd, sd->iov[0].iov_base, sd->iov0_pos, 0);
#endif
    err = http_parse_header(sd, 0);
    if (err < 0) {
      fprintf(stderr, "%s:%d.\n", __FILE__, __LINE__);
      err = 0;
      goto yield;
    }
  } else {
    /*
    fprintf(stderr, "More data needed to proceed.\n");
    */
    err = 2;
    goto yield;
  }

  if (err < 0) goto yield;
#if 0
  buf[sizeof(buf)-1] = '\0';
  parse_header(buf, temp, 0);
#endif

  /* fprintf(stdout, "Header OK\n"); */
  /* now generate response */
#if 0
  pr = ss->prog[hs->fd%ss->nworkers];
  po = ss->pout[hs->fd%ss->nworkers];
  prog_out_reset(po);
#endif

#if 0
  prog_exec(pr, po); /* Execute PHP Engine */
#endif
  //prog_exec(pr, NULL); /* Execute PHP Engine */
  //fprintf(stderr, "%s\n", po->str);

//#if 0
  rresult = 0;
  lrand48_r(&ss->rbuf[hs->fd%ss->nworkers], &rresult);
  //int clen = sprintf(contents, "Random Token: %0lx", rresult);
  int clen = sprintf(contents, "Token: %0lx", rresult);
//#endif
  //int clen = po->len;
  //contents = po->str;
  char *ctype = "text/plain";
  size_t resp_hdr_len = HTTP_RESP_HDR_GEN(buf,200,"OK",clen,ctype);
  if (sd->iov[0].iov_len < resp_hdr_len) {
    fprintf(stderr, "Expanding space in iov[0], newsize = %lu\n", resp_hdr_len);
    ptr = realloc(sd->iov[0].iov_base, resp_hdr_len);
    if (ptr) {
      sd->iov[0].iov_base = ptr;
      sd->iov0_len = sd->iov[0].iov_len = resp_hdr_len;
    } else {
      err = -1;
      goto yield;
    }
  }
  memcpy(sd->iov[0].iov_base, buf, resp_hdr_len);
  sd->iov0_pos = sd->iov[0].iov_len = resp_hdr_len;
  if (sd->iov1_len < clen) {
    ptr = realloc(sd->iov[1].iov_base, clen);
    if (ptr) {
      sd->iov[1].iov_base = ptr;
      sd->iov1_len = sd->iov[1].iov_len = clen;
    } else {
      err = -1;
      goto yield;
    }
  }
  memcpy(sd->iov[1].iov_base, contents, clen);
  sd->iov1_pos = sd->iov[1].iov_len = clen;

  tlen = sd->iov[0].iov_len + sd->iov[1].iov_len;
  nwritten = writev(hs->fd, sd->iov, 2);
  if (nwritten < tlen) {
    fprintf(stderr, "Less data written!\n");
    err = -1;
    goto yield;
  }

  err = 0; /* wait for more requests (keep-alive) from this client */

yield:
  switch (err) {
    case 0:
      sd->iov0_pos = 0;
      sd->iov1_pos = 0;
      break;
    case -1:
      http_session_close(hs,(system_t *) sys);
      break;
    default:
      err = 0;
      break;
  }

  if (contents) free(contents);

  return NULL;
}


static void *http_on_data(void *self, void *sys)
{
  void *ret = NULL;
  http_session_t *s = self;
  if (CHECKFLAG(s->ev.events, EPOLLRDHUP) ||
      CHECKFLAG(s->ev.events, EPOLLHUP) ||
      CHECKFLAG(s->ev.events, EPOLLERR)
     ) {
    http_session_close(s,(system_t *) sys);
    ret = NULL;
  } else {
    /*
    int x = s->fd % NWORKERS;
    */
    http_session_data_t *sd = s->data;
    if (!sd) {
      sd = http_session_data_new();
    }
    if (sd && !sd->initialized) {
      sd->iov[0].iov_base = calloc(1, HTTP_MAXHEADER+1);
      sd->iov[0].iov_len = HTTP_MAXHEADER+1;
      sd->iov[1].iov_base = calloc(1, 512);
      sd->iov[1].iov_len = 512;
      sd->initialized = 1;

      sd->iov0_len = sd->iov[0].iov_len;
      sd->iov1_len = sd->iov[1].iov_len;
    }
    s->data = sd;
    ret = s;
  }

  return ret;
}


static void *http_on_connect(void *self, void *sys)
{
  int connfd;
  int value = 1;
  struct sockaddr_in local;
  socklen_t addrlen = sizeof(local);
  hook_t *h = (hook_t *) self;
  http_session_t *s;
  while ((connfd = ACCEPT(h->fd, (struct sockaddr *) &local, &addrlen)) > 0) {
    http_session_new(s, (system_t *) sys, connfd);
    if (s) {
      setsockopt(connfd, IPPROTO_IP, TCP_NODELAY, &value, sizeof(value));
      s->fd = connfd;
      *(&s->ready) = http_on_data;
      *(&s->process) = http_process_data;
      s->ev.events = EPOLLIN | EPOLLRDHUP | EPOLLPRI | EPOLLET;
      s->ev.data.ptr = s;
      EVQ_ADD(((system_t *) sys)->eventfd, s->fd, &s->ev);
    } else {
      close(connfd);
    }
  }

  /*
   * the following will be performed by a watchdog carefully when the system
   * is idle
   */
  /* if we allow freeing then we got one producer two consumers
   * (this and this) and there is unstable condition in the lock-free queue
   * so the following code in this function is ifdef'd 0 (don't compile)
   */
#if 0

  int i;
  int n = ((system_t *) (sys))->nworkers;
  for (i = 0; i < n; ++i) {
    while ((s = lfq_dequeue(((system_t *) (sys))->fq[i]))) {
      http_session_free_real(s);
    }
  }
#endif

  return NULL;
}

#if 0
system_t *mod_http_init(system_t *sys)
{
  struct sockaddr_in addr;
#ifdef __linux__
  int type = SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC;
#else
  int type = SOCK_STREAM;
#endif

  int sockfd = socket(AF_INET, /* family */
      type, /* type */
      0); /* protocol */
  if (sockfd == -1) {
    err_warn("socket");
    return sys;
  }
  int one = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
#if 0
  struct linger ling = { 1, 1 };
  setsockopt(sys->aws.sockfd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling)); /* graceful */
  one = 1;
  setsockopt(sys->aws.sockfd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
#endif

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(HTTP_PORT);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
    err_warn("bind");
    close(sockfd);
    sockfd = -1;
    return sys;
  }

#ifndef BACKLOG
  /* check /proc/sys/net/core/somaxconn */
#define BACKLOG 100000
#endif
  if (listen(sockfd, BACKLOG) == -1) { /* 128: backlog */
    err_warn("listen");
    close(sockfd);
    sockfd = -1;
    return sys;
  }

  http_session_t *s = http_session_new();
  s->fd = sockfd;
  s->ready = http_on_connect;
  s->ev.events = EPOLLIN;
  s->ev.data.ptr = s;

  EVQ_ADD(sys->eventfd, sockfd, &s->ev);

  return sys;
}
#endif

int module_start(system_t *sys)
{
  struct sockaddr_in addr;
#ifdef __linux__
  int type = SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC;
#else
  int type = SOCK_STREAM;
#endif

  int sockfd = socket(AF_INET, /* family */
      type, /* type */
      0); /* protocol */
  if (sockfd == -1) {
    err_warn("socket");
    return -1;
  }
  int one = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
#if 0
  struct linger ling = { 1, 1 };
  setsockopt(sys->aws.sockfd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling)); /* graceful */
  one = 1;
  setsockopt(sys->aws.sockfd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
#endif

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(HTTP_PORT);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
    err_warn("bind");
    close(sockfd);
    sockfd = -1;
    return -1;
  }

#ifndef BACKLOG
  /* check /proc/sys/net/core/somaxconn */
#define BACKLOG 100000
#endif
  if (listen(sockfd, BACKLOG) == -1) { /* 128: backlog */
    err_warn("listen");
    close(sockfd);
    sockfd = -1;
    return -1;
  }

  hook_t *s = hook_new();
  if (s) {
    s->fd = sockfd;
    s->ready = http_on_connect;
    s->ev.events = EPOLLIN;
    s->ev.data.ptr = s;
  } else {
    fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
    exit(EXIT_FAILURE);
  }


  /*
   * load other modules
   */
#if 0
  hrq_init();
#endif


  EVQ_ADD(sys->eventfd, sockfd, &s->ev);

  return sockfd;
}

int module_stop(system_t *sys, module_t *mod)
{
  if (mod && mod->fd > 0) close(mod->fd);
  /*
   * closing the descriptor automatically removes it from the event
  EVQ_DEL(sys->eventfd, mod->fd);
  */
  return 0;
}


