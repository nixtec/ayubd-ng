/*
 * mod_http.c
 * http module
 */
#include "mod_http.h"

/* Callbacks should return non-zero to indicate an error. The parser will
 * then halt execution.
 *
 * The one exception is on_headers_complete. In a HTTP_RESPONSE parser
 * returning '1' from on_headers_complete will tell the parser that it
 * should not expect a body. This is used when receiving a response to a
 * HEAD request which may contain 'Content-Length' or 'Transfer-Encoding:
 * chunked' headers that indicate the presence of a body.
 *
 * Returning `2` from on_headers_complete will tell parser that it should not
 * expect neither a body nor any futher responses on this connection. This is
 * useful for handling responses to a CONNECT request which may not contain
 * `Upgrade` or `Connection: upgrade` headers.
 *
 * http_data_cb does not return data chunks. It will be called arbitrarily
 * many times for each string. E.G. you might get 10 callbacks for "on_url"
 * each providing just a few characters more data.
 */

/*
 * in http_parser.h following callback types are defined
 * typedef int (*http_data_cb) (http_parser*, const char *at, size_t length);
 * typedef int (*http_cb) (http_parser*);
 */

#define hr_conn "keep-alive"
#define hr_resp_iov0 "HTTP/1.1 %d %s\r\n" \
  "Server: nixtec-ws/0.9/ayub\r\n" \
  "Content-Length: %ld\r\n" \
  "Connection: " hr_conn "\r\n" \
  "Content-Type: %s\r\n\r\n"
#define UD_MOD_HS (UD_RESERVED+1)
#define UD_MOD_HP (UD_RESERVED+2)
#define UD_MOD_REQ (UD_RESERVED+3)
#define UD_MOD_RESP (UD_RESERVED+4)

#define D_MOD_REQ_URI 0		/* URI */
#define D_MOD_REQ_URI_SZ IOQUEUE_CHUNK	/* URI size */
#define D_MOD_REQ_ULEN 1	/* URI length */
#define D_MOD_REQ_PATH 2	/* PATH */
#define D_MOD_REQ_PATH_SZ IOQUEUE_CHUNK	/* PATH size */
#define D_MOD_REQ_PLEN 3	/* PATH length */
#define D_MOD_REQ_QUERY 4	/* QUERY STRING */
#define D_MOD_REQ_QUERY_SZ IOQUEUE_CHUNK	/* QUERY SIZE */
#define D_MOD_REQ_QLEN 5	/* QUERY STRING length */
#define D_MOD_REQ_HDR 6		/* RAW HEADER */
#define D_MOD_REQ_HLEN 7	/* RAW HEADER LENGTH */
#define D_MOD_REQ_SCRATCH 8
#define D_MOD_REQ_SCRATCH_SZ (IOQUEUE_CHUNK*1)
#define D_MOD_REQ_HOOKPTR 9
#define D_MOD_REQ_SZ 10	/* to next aligned data size */

#define D_MOD_RESP_CODE 0
#define D_MOD_RESP_STR 1
#define D_MOD_RESP_LEN 2
#define D_MOD_RESP_SCRATCH 3
#define D_MOD_RESP_SCRATCH_SZ (IOQUEUE_CHUNK*1)
#define D_MOD_RESP_SZ 4		/* to next aligned data size */



#ifdef USE_HS
static int eventHandler(unsigned int id, unsigned long long from, unsigned long long to, unsigned int flags, void *ctx)
{
  fprintf(stderr, "Match for pattern at offset %llu-%llu\n", from, to);
  assert(write(STDERR_FILENO, ctx+from, to-from) == (to-from));
  assert(write(STDERR_FILENO, "\n", 1) == 1);
  return 0;
}
#endif

int on_message_begin_cb(http_parser *p)
{
  //fprintf(stderr, "%s\n", __func__);
  return 0;
}

int on_url_cb(http_parser *p, const char *buf, size_t len)
{

  value_t *req = p->data;
  len = MIN(len, IOQUEUE_CHUNK-1); /* make sure we don't overflow the url buffer */
  char *str = (typeof(str)) req[D_MOD_REQ_URI];
  memcpy(str, buf, len);
  *(str+len) = '\0';
  req[D_MOD_REQ_ULEN] = len;


#if 0
  fprintf(stderr, "%s\n", __func__);
#endif

#if 0
  char *ptr = _aget((array_t *)p->data, "url");
  len = MIN(len, HTTP_HEADER_VALSZ-1);
  memcpy(ptr, buf, len);
  *(ptr+len) = '\0';
#endif

  //hook_t *hp = (hook_t *) p->data;
  //int n = (int) _aget(hp->data, "num");
  //system_t *sys = (system_t *) _aget(hp->data, "sys");
#ifdef USE_HS
  if (hs_scan(sys->hs_database[n], buf, len, 0, sys->hs_scratch[n], eventHandler, (void *) buf) != HS_SUCCESS) {
    fprintf(stderr, "Error: Unable to scan input buffer.\n");
  }

#if 0
  buf = "/captcha/get";
  len = strlen(buf);
  //char *pat = (char *) _aget(sys->symtbl, "http.route");
  if (hs_scan(sys->hs_database[n], buf, len, 0, sys->hs_scratch[n], eventHandler, (void *) buf) != HS_SUCCESS) {
    fprintf(stderr, "Error: Unable to scan input buffer.\n");
  }
#endif

#endif

#if 0
  assert(write(STDERR_FILENO, buf, len) == len);
  assert(write(STDERR_FILENO, "\n", 1) == 1);
#endif

#if 0
  if (len == 5 && strncmp(buf, "/stat", len) == 0) {
    if (p->data) {
      system_t *sys = (system_t *) _aget(((hook_t *) p->data)->data, "sys");
      if (sys) {
	int n = (int) _aget(sys->symtbl, "sys.nworkers");
	int i;
	int sum = 0;
	char key[20];
	int x = 0;
	for (i = 0; i < n; i++) {
	  sprintf(key, "http.req/%i", i);
	  x = (int) _aget(sys->symtbl, key);
	  fprintf(stderr, "i=%d, x=%d\n", i, x);
	  sum += x;
	}
	fprintf(stderr, "Total Requests Served: %d\n", sum);
      }
    }
  }
#endif

  return 0;
}

int on_status_cb(http_parser *p, const char *buf, size_t len)
{
  //fprintf(stderr, "%s\n", __func__);
  return 0;
}

int on_header_field_cb(http_parser *p, const char *buf, size_t len)
{

/*
  char tbuf[HTTP_HEADER_KEYSZ];
  len = MIN(len, sizeof(tbuf));
  _aset(req, "url", DO_MALLOC(HTTP_HEADER_VALSZ));
  */

#if 0
  fprintf(stderr, "%s\n", __func__);
  assert(write(STDERR_FILENO, buf, len) == len);
  assert(write(STDERR_FILENO, "\n", 1) == 1);
#endif

  return 0;
}

int on_header_value_cb(http_parser *p, const char *buf, size_t len)
{

#if 0
  fprintf(stderr, "%s\n", __func__);
  assert(write(STDERR_FILENO, buf, len) == len);
  assert(write(STDERR_FILENO, "\n", 1) == 1);
#endif

  return 0;
}

int on_headers_complete_cb(http_parser *p)
{
  //fprintf(stderr, "%s\n", __func__);
  return 0;
}

int on_body_cb(http_parser *p, const char *buf, size_t len)
{
  //fprintf(stderr, "%s\n", __func__);
  return 0;
}

int on_message_complete_cb(http_parser *p)
{
  //fprintf(stderr, "%s\n", __func__);
  return 0;
}

int on_chunk_header_cb(http_parser *p)
{
  //fprintf(stderr, "%s\n", __func__);
  return 0;
}

int on_chunk_complete_cb(http_parser *p)
{
  //fprintf(stderr, "%s\n", __func__);
  return 0;
}






/*
int my_http_cb(http_parser *p)
{
  return 0;
}

int my_http_data_cb(http_parser *p, const char *buf, size_t len)
{
  assert(write(1, buf, len) == len);
  assert(write(1, "\n", 1) == 1);
  return 0;
}
*/

int hook_on_connect_test(void *data)
{
  fprintf(stderr, "%s\n", __func__);
//  char buf[128];
  hook_t *hp = data;
  //struct msghdr *imsg = (struct msghdr *) _aget(hp->data, "msghdr");
  struct msghdr *imsg = (typeof (imsg)) hp->udata[UD_CORE_MSGHDR];
  struct sockaddr_storage *addr = (struct sockaddr_storage *) imsg->msg_name;
  char namebuf[256];
  char portbuf[256];
  int flags = NI_NUMERICHOST | NI_NUMERICSERV;

  if (getnameinfo((struct sockaddr *) addr, sizeof(*addr), namebuf, sizeof(namebuf), portbuf, sizeof(portbuf), flags) == 0) {
    fprintf(stderr, "Accepted Connection from %s:%s\n", namebuf, portbuf);
  }

  return 1;
}


#if 0
void captcha_get(const char *req_path, array_t *req_query, array_t *req_header, const char *req_body, int *resp_code, char **resp_str, size_t *resp_strlen, void *data)
{
  if (resp_str && resp_code && resp_strlen) {
    *resp_str = strdup("Hello World [captcha_get]!");
    *resp_code = 200;
    *resp_strlen = strlen(*resp_str);
  }
//  fprintf(stderr, "%s\n", __func__);
}

void captcha_verify(const char *req_path, array_t *req_query, array_t *req_header, const char *req_body, int *resp_code, char **resp_str, size_t *resp_strlen, void *data)
{
  if (resp_str && resp_code && resp_strlen) {
    *resp_str = strdup("Hello World! [captcha_verify]");
    *resp_code = 200;
    *resp_strlen = strlen(*resp_str);
  }
//  fprintf(stderr, "%s\n", __func__);
}
#endif

void captcha_get(value_t *req, value_t *resp, void *data)
{
  if (req && resp) {
    char *str = (typeof(str)) resp[D_MOD_RESP_SCRATCH];
    //fprintf(stderr, "%s [%s]\n", __func__, (char *) req[D_MOD_REQ_PATH]);
    int32_t rnd = 0;
#ifdef USE_RAND
    hook_t *hp = (typeof(hp)) req[D_MOD_REQ_HOOKPTR];
    RANDOM(hp, rnd);
#endif
    /*
    struct timeval tv;
    gettimeofday(&tv, NULL);
    size_t len = MIN(sprintf(str, "PATH [%s], RND [%d], TV [%ld,%ld]!", (char *) req[D_MOD_REQ_PATH], rnd, tv.tv_sec, tv.tv_usec), D_MOD_RESP_SCRATCH_SZ-1);
    */
    size_t len = MIN(sprintf(str, "PATH [%s], RND [%d]!", (char *) req[D_MOD_REQ_PATH], rnd), D_MOD_RESP_SCRATCH_SZ-1);
    *(str+len) = '\0'; /* safety NUL byte */

    resp[D_MOD_RESP_CODE] = 200;
    resp[D_MOD_RESP_STR] = (value_t) str;
    resp[D_MOD_RESP_LEN] = len;
  }
}

void captcha_verify(value_t *req, value_t *resp, void *data)
{
  if (req && resp) {
    char *str = (typeof(str)) resp[D_MOD_RESP_SCRATCH];
    //fprintf(stderr, "%s [%s]\n", __func__, (char *) req[D_MOD_REQ_PATH]);
    int32_t rnd = 0;
#ifdef USE_RAND
    hook_t *hp = (typeof(hp)) req[D_MOD_REQ_HOOKPTR];
    RANDOM(hp, rnd);
#endif
    size_t len = MIN(sprintf(str, "PATH [%s], RND [%d]!", (char *) req[D_MOD_REQ_PATH], rnd), D_MOD_RESP_SCRATCH_SZ-1);
    *(str+len) = '\0'; /* safety NUL byte */

    resp[D_MOD_RESP_CODE] = 200;
    resp[D_MOD_RESP_STR] = (value_t) str;
    resp[D_MOD_RESP_LEN] = len;
  }
//  fprintf(stderr, "%s\n", __func__);
}


/* caller will free 'out' */
/* return value is length of 'out' (otherwise set 0) */
int http_on_data_default(void *data)
{
  hook_t *hp = data;

  //struct msghdr *imsg = (typeof(imsg)) _aget(hp->data, "msghdr");
  struct msghdr *imsg = (typeof (imsg)) hp->udata[UD_CORE_MSGHDR];
  assert(imsg != NULL);

  assert(imsg->msg_iovlen == 2); /* for safety reasons (will make it dynamic after result publication event) */

  int ret = 0;
  ssize_t nread, nparsed;

  /*
  const char *key;
  char tkey[20];
  */


/*
  array_t *req = array_new();
  _aset(req, "url", DO_MALLOC(HTTP_HEADER_VALSZ));
  array_t *resp = array_new();
  */

  http_parser_settings *settings;
  if (!(settings = (typeof(settings)) hp->udata[UD_MOD_HS])) {
    hook_malloc(hp, settings, sizeof(*settings));
    http_parser_settings_init(settings);
    settings->on_url = on_url_cb;
    // For now I am not doing action on headers, so it's not required for current situation (result)
    //settings->on_header_field = on_header_field_cb;
    //settings->on_header_value = on_header_value_cb;
    hp->udata[UD_MOD_HS] = (unsigned long int) settings;
    //settings->on_headers_complete = on_headers_complete_cb;
    //_aset(hp->data, key, settings);
  }

  value_t *req = NULL;
  if ((req = (typeof(req)) hp->udata[UD_MOD_REQ]) == NULL) {
    hook_zalloc(hp, req, D_MOD_REQ_SZ * sizeof(*req));
    hp->udata[UD_MOD_REQ] = (unsigned long int) req;

    hook_zalloc(hp, req[D_MOD_REQ_URI], D_MOD_REQ_URI_SZ);
    hook_zalloc(hp, req[D_MOD_REQ_PATH], D_MOD_REQ_PATH_SZ);
    hook_zalloc(hp, req[D_MOD_REQ_QUERY], D_MOD_REQ_QUERY_SZ);
    hook_zalloc(hp, req[D_MOD_REQ_SCRATCH], D_MOD_REQ_SCRATCH_SZ);
    req[D_MOD_REQ_HOOKPTR] = (unsigned long int) hp;
  }



  http_parser *parser;
  if (!(parser = (typeof(parser)) hp->udata[UD_MOD_HP])) {
    hook_malloc(hp, parser, sizeof(*parser));
    hp->udata[UD_MOD_HP] = (unsigned long int) parser;
  }
  http_parser_init(parser, HTTP_REQUEST);
  parser->data = req;


  nread = (typeof(nread)) hp->udata[UD_CORE_NREAD];

  /* we are only taking first buffer from vector, hopefully the whole header fits in this */
  nparsed = http_parser_execute(parser, settings, imsg->msg_iov[0].iov_base, nread);
  if (parser->upgrade) {
    fprintf(stderr, "Upgrade Requested, not implemented yet!\n");
  } else if (nparsed != nread) {
    fprintf(stderr, "Not Completely parsed.\n");
  }


/*
  system_t *sys = (system_t *) _aget(hp->data, "sys");
  int  i = _aget(hp->data, "num");
  sprintf(tkey, "http.req/%d", i);
  _aset(sys->symtbl, tkey, _aget(sys->symtbl, tkey)+1);
*/

#ifdef USE_REDIS
  //sys->redis_reply[i] = redisCommand(sys->redis_ctx[i], "PING");

  //sys->redis_reply[i] = redisCommand(sys->redis_ctx[i], "SET foo bar");
  //fprintf(stderr, "SET[%d]: %s\n", i, sys->redis_reply[i]->str);
  //freeReplyObject(sys->redis_reply[i]);

  //sys->redis_reply[i] = redisCommand(sys->redis_ctx[i], "GET foo");
  //fprintf(stderr, "GET[%d]: %s\n", i, sys->redis_reply[i]->str);
  //freeReplyObject(sys->redis_reply[i]);
#endif


  // now we parse url to extract information
  struct http_parser_url hurl;

  char *uri = (typeof(uri)) req[D_MOD_REQ_URI];
  size_t ulen = req[D_MOD_REQ_ULEN]; /* parsed length */
  char *path = (typeof(path)) req[D_MOD_REQ_PATH];
  size_t pathlen = D_MOD_REQ_PATH_SZ; /* initial max size */
  char *query = (typeof(query)) req[D_MOD_REQ_QUERY];
  size_t qlen = D_MOD_REQ_QUERY_SZ; /* initial max size */

  http_route_action_t action = NULL;

  int resp_code = 200;
  char *resp_str = NULL;
  size_t resp_strlen = 0;

  /* using associative array for req and resp degrades the performance, i think it's due to using string operations for array keys */
  value_t *resp = NULL;
  array_t *route = NULL;
  if ((resp = (typeof(resp)) hp->udata[UD_MOD_RESP]) == NULL) {
    hook_zalloc(hp, resp, D_MOD_RESP_SZ * sizeof(*resp));
    hp->udata[UD_MOD_RESP] = (unsigned long int) resp;
    hook_zalloc(hp, resp[D_MOD_RESP_SCRATCH], D_MOD_RESP_SCRATCH_SZ);
  }


  if ((route = (typeof(route)) hp->udata[UD_MOD_HTTP_ROUTE]) != NULL) {
    http_parser_url_init(&hurl);
    if (http_parser_parse_url(uri, ulen, 0, &hurl) == 0) {
      //write(1, uri->str+hurl.field_data[UF_PATH].off, hurl.field_data[UF_PATH].len);
      //write(1, "\n", 1);
      if (hurl.field_data[UF_PATH].len > 0) {
	pathlen = MIN(hurl.field_data[UF_PATH].len, pathlen-1);
	memcpy(path, uri+hurl.field_data[UF_PATH].off, pathlen);
	*(path+pathlen) = '\0';
	req[D_MOD_REQ_PLEN] = pathlen;

	qlen = MIN(hurl.field_data[UF_QUERY].len, qlen-1);
	if (qlen > 0) {
	  memcpy(query, uri+hurl.field_data[UF_QUERY].off, qlen);
	  *(query+qlen) = '\0';
	  req[D_MOD_REQ_QLEN] = qlen;
	}



	if ((action = (http_route_action_t) _aget(route, path)) != NULL) {
	  //void captcha_verify(const char *req_path, array_t *req_query, array_t *req_header, const char *req_body, int *resp_code, char **resp_str, size_t *resp_strlen, void *data)
	  //action(path, query, rhead, NULL, &resp_code, &resp_str, &resp_strlen, NULL);
	  action(req, resp, NULL);
	  //fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
	  resp_code = (typeof(resp_code)) resp[D_MOD_RESP_CODE];
	  //resp_code = _aget(resp, "_code");
	  resp_str = (typeof(resp_str)) resp[D_MOD_RESP_STR];
	  //resp_str = (typeof(resp_str)) _aget(resp, "_str");
	  resp_strlen = (typeof(resp_strlen)) resp[D_MOD_RESP_LEN];
	  //resp_strlen = _aget(resp, "_len");
	} else {
	  resp_str = "No action defined against route. Please contact support.";
	  resp_code = 403;
	  resp_strlen = strlen(resp_str);
	}
	//write(1, uri->str+hurl.field_data[UF_QUERY].off, hurl.field_data[UF_QUERY].len);
	//write(1, "\n", 1);
      }
    }
  } else {
    resp_str = "No route defined. Please contact support.";
    resp_code = 503;
    resp_strlen = strlen(resp_str);
  }

  if (resp_strlen > 0) {
    //ret += IOV_PRINTF(&imsg->msg_iov[1], "%s", "No data handler specified. Please contact support.");
    //ret += IOV_PRINTF(&imsg->msg_iov[1], "Real Time Counter: %d", (int) _aget(sys->symtbl, tkey));
    //ret += IOV_PRINTF(&imsg->msg_iov[1], "%s", uri->str);
    ret += IOV_PRINTF(&imsg->msg_iov[1], "%s", resp_str);
    //ret += IOV_PRINTF(&imsg->msg_iov[1], "%s", "Hello World!"); // if the content length changes, 'ab' considers it an error
    ret += IOV_PRINTF(&imsg->msg_iov[0], hr_resp_iov0, resp_code, http_status_str(resp_code), imsg->msg_iov[1].iov_len, "text/plain");
  }

  //fprintf(stderr, "%d bytes to be written\n", ret);

  return ret;
}


int hook_on_data_test(void *data)
{
  int ret = 0;
  ssize_t n = 0;
//  hook_t *hp = data;
//  struct msghdr *imsg = (typeof(imsg)) _aget(((hook_t *)data)->data, "msghdr");
  n = ioqueue_socket_recvmsg(data, 0);
  ((hook_t *) data)->udata[UD_CORE_NREAD] = n; /* so that on http_parser we can know how many bytes read */

  //fprintf(stderr, "%s\n", __func__);

  if (n <= 0) goto cleanup;

  if (http_on_data_default(data) > 0) {
    n = ioqueue_socket_sendmsg(data, 0);
    ret = n;
  }

cleanup:

  return ret;
}

int hook_on_close_test(void *data)
{
  fprintf(stderr, "%s\n", __func__);
  return 1;
}

int module_start(system_t *sys)
{
  int ret = -1;

  array_t *args = array_new();
  /* module can pass arbitrary information through this array */

  ioqueue_t *iq = ioqueue_new(0); /* by default tcp socket to be used */
  if (iq) {
#ifdef USE_UNIX_SOCKET
    ioqueue_set_flags(iq, IOQUEUE_SOCKET_UNIX);
    _aset(args, "node", "/tmp/ayubd-ng.sock");
    //ret = ioqueue_socket_init(iosock, "/tmp/ayubd-ng.sock", NULL);
#else
    //_aset(args, "node", "0.0.0.0");
    _aset(args, "node", "localhost");
    _aset(args, "serv", "9990");
#endif
    _aset(args, "sys", sys);
    //_aset(args, "ioq", iq);

#ifdef USE_HOCO
    _aset(args, "hoco", hook_on_connect_test);
#endif
#ifdef USE_HOCL
    _aset(args, "hocl", hook_on_close_test);
#endif

    _aset(args, "hoda", hook_on_data_test); /* this is required, otherwise socket connection will be closed immediately */


    array_t *routes = array_new(); // routes array
    _aset(routes, "/app/get", captcha_get);
    _aset(routes, "/app/verify", captcha_verify);

    _aset(args, "route", routes);

    //array_t *apps = array_new(); // apps array
    //_aset(apps, "captcha", acts);
    //_aset(sys->symtbl, "http.apps", apps);


    ret = ioqueue_socket_prepare(iq, args);
    if (ret != 0) {
      ret = -1;
      goto cleanup;
    }

    ret = ioqueue_socket_deploy(iq, args);
    if (ret != 0) {
      ret = -1;
      goto cleanup;
    }
  }

cleanup:

  if (ret == -1) array_free(args); /* free only if error in prepare/deploy, the ioqueue might need args later */
  return ret;
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

