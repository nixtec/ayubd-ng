/*
 * mod_http.h
 * keep-alive keeps keep-alive enabled clients happy and performs well too
 */
#ifndef __MOD_HTTP_H__
#define __MOD_HTTP_H__

#define _GNU_SOURCE

#include <string.h>
#include <stdlib.h>
#include <time.h>		/* time_t, time(2), etc. */
#include <sys/uio.h>		/* writev(2), struct iovec, etc. */
#include <sys/socket.h>
#ifdef USE_UDS
#include <sys/un.h>
#include <sys/file.h>
#endif
#include <netinet/in.h>
#include <netinet/tcp.h>
#if 0
#include "../../main.h"
#endif
#include "../../module.h"
#include "../../ioqueue.h"
#include "../../ioqueue_socket.h"

#include <http_parser.h>

#if 0
#include "../../prog_lua.h"
#endif

#ifndef HTTP_KEEPALIVE_TIMEOUT
#define HTTP_KEEPALIVE_TIMEOUT 15	/* seconds */
#endif

/* 's' must NOT have any side effects */
#define HTTP_VERIFY_METHOD(s,l) ((l) >=3 && (*(s) == 'G' && *((s)+1) == 'E' && *((s)+2) == 'T') ? __K__ : ERROR)
/* strcmp() version is slower than above */
/* #define VERIFY_METHOD(s) (strcmp(s, "GET") == 0 ? __K__ : ERROR) */

#define MAX_HEADERS 100		/* maximum number of headers allowed */
typedef struct {
  uint8_t keyidx;
  char *value;
} http_header_t;

typedef struct {
  state_t *state; /* allocate (only if needed, using state_new()) (jump buffer) */
  struct iovec iov[2];
  size_t iov0_pos;
  size_t iov0_len;
  size_t iov1_pos;
  size_t iov1_len;
  time_t heartbeat;
  int initialized;

  http_parser_settings settings;
  http_parser *parser;

  //uint8_t header_loc[MAX_HEADERS]; /* header locations */
  //char *header_val[MAX_HEADERS];

  //uint8_t board;
  //int32_t roll;
} http_session_data_t;

typedef hook_t http_session_t;

#define http_session_new(hs,sys,fd) do {			\
  hs = lfq_dequeue((sys)->fq[fd%(sys)->nworkers]);		\
  if (!hs) hs = hook_new();					\
} while (0)

#if 0
#define http_session_new() hook_new()
#endif
#define http_session_data_new() DO_CALLOC(1, sizeof(http_session_data_t))
#define http_session_free(s,sys) do {				\
  if (s) {							\
    int x = (s)->fd % (sys)->nworkers;				\
    (s)->fd = 0;						\
    (s)->ready = NULL;						\
    (s)->process = NULL;					\
    http_session_data_t *sd = (s)->data;			\
    if (sd) {							\
      if (sd->state) memset(sd->state, 0, sizeof(state_t));	\
      sd->iov0_pos = sd->iov1_pos = 0;				\
      memset(sd->parser, 0, sizeof(http_parser)); 		\
    }								\
    lfq_enqueue(((system_t *) (sys))->fq[x], (s));		\
  }								\
} while (0)
#define http_session_free_real(s) do {				\
  if (s) {							\
    http_session_data_t *sd = (s)->data;			\
    if (sd) {							\
      if ((sd)->state) DO_FREE((sd)->state);			\
      if ((sd)->iov[0].iov_base) DO_FREE((sd)->iov[0].iov_base);	\
      if ((sd)->iov[1].iov_base) DO_FREE((sd)->iov[1].iov_base);	\
      if ((sd)->parser) DO_FREE((sd)->parser);			\
      DO_FREE(sd);						\
    }								\
    DO_FREE(s);							\
    s = NULL;							\
  }								\
} while (0)

#define http_session_close(s,sys) do {				\
  close((s)->fd);						\
  http_session_free(s,sys);					\
} while (0)

#if 0
#define http_session_close(s) do {				\
  close((s)->fd);						\
  http_session_data_t *sd = (http_session_data_t *) (s)->data;	\
  if (sd) {							\
    if ((sd)->state) { DO_FREE((sd)->state); sd = NULL; }		\
    if ((sd)->iov[0].iov_base) DO_FREE((sd)->iov[0].iov_base);	\
    if ((sd)->iov[1].iov_base) DO_FREE((sd)->iov[1].iov_base);	\
    DO_FREE(sd);							\
  }								\
  DO_FREE(s);							\
} while (0)
#endif

#if 0
/* caller must free the decoded value */
static char *rawurldecode(const char *url)
{
  if (!url || *url == '\0')
    return NULL;

  size_t len = strlen(url);
  char *decoded = DO_MALLOC(len+1);
  char *tmp = decoded;
  char x, y;

  while(*url) {
    if (*url != '%') {
      *tmp++ = *url++;
    } else if (*(url+1) && *(url+2)) {
      url++;
      x = *url++;
      y = *url++;
      *tmp++ = HEXCHARVAL(x) * 16 + HEXCHARVAL(y);
    } else {
      *tmp++ = *url++;
    }
  }

  *tmp = '\0';

  return decoded;
}
#endif

#if 0
static void http_elem_cb(void *data, const char *at, size_t len)
{
  DUMP(at,len);
  return;
}
#endif


#ifndef HTTP_MAXHEADER
#define HTTP_MAXHEADER 4096
#endif

#if 0
static char hr_about_iov1[] = "Developed and Maintained by Ayub <ayub@nixtecsys.com>, Nixtec Systems";
#endif


#if 0
static char hr_about_iov1[] = "Test";
#define hr_about_iov1_len (sizeof(hr_about_iov1)-1)
static char hr_about_iov0[] = "HTTP/1.1 200 OK\r\n" \
"Server: aws/0.9\r\n" \
"Content-Length: 4\r\n" \
"Connection: keep-alive\r\n" \
"Content-Type: text/plain\r\n\r\n";
#define hr_about_iov0_len (sizeof(hr_about_iov0)-1)
#define HTTP_RESPONSE_ABOUT_IOV0_LEN LEN(HTTP_RESPONSE_ABOUT_IOV0)
#endif

#if 0
#define hr_conn "keep-alive"
static char hr_resp_iov0[] = "HTTP/1.1 %d %s\r\n" \
"Server: nixtec-ws/0.9/ayub\r\n" \
"Content-Length: %ld\r\n" \
"Connection: " hr_conn "\r\n" \
"Content-Type: %s\r\n\r\n";
#define HTTP_RESP_HDR_GEN(buf,resp_code,resp_code_str,content_len,content_type) sprintf(buf, hr_resp_iov0, resp_code, resp_code_str, content_len, content_type)
#endif

#if 0
#define HTTP_RESPONSE "HTTP/1.1 200 OK\r\n" \
"Server: aws/0.9\r\n" \
"Content-Length: 12\r\n" \
"Connection: keep-alive\r\n" \
"Content-Type: text/plain\r\n\r\n" \
"Hello World!"
#define HTTP_RESPONSE_LENGTH sizeof(HTTP_RESPONSE)
#endif


#define DUMP(at,len) do { fprintf(stderr, "%s\n", __func__); fwrite(at, sizeof(char), len, stderr); fprintf(stderr, "\n"); } while (0)
#define DUMPX(field,flen,value,vlen) do { fprintf(stderr, "%s\n", __func__); fwrite(field, sizeof(char), flen, stderr); fprintf(stderr, "\n"); fwrite(value, sizeof(char), vlen, stderr); fprintf(stderr, "\n"); } while (0)

typedef struct {
  char *rname;
  void (*cb)(char *field, size_t flen);
} http_req_t;


typedef struct {
  char *str;
  size_t len;
} http_uri_t;

//typedef void (*http_route_action_t)(const char *req_path, array_t *req_query, array_t *req_header, const char *req_body, int *resp_code, char **resp_str, size_t *resp_strlen, void *data);
typedef void (*http_route_action_t)(value_t *req, value_t *resp, void *data);

#define HTTP_HEADER_KEYSZ 32
#define HTTP_HEADER_VALSZ 512


#endif /* !__MOD_HTTP_H__ */
