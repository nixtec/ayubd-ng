/*
 * str_tokens.c
 * string tokenizer
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <fcntl.h>
#include <unistd.h>
#include <pcre.h>

typedef struct {
  const char *ptr;
  size_t len;
} token_t;


typedef struct {
  char *ptr;
  size_t len;
} string_t;

typedef void (*free_func_t)(void *);

/* flat list (just array, we're calling it list to be functional */
typedef struct {
  void *ptr;
  size_t len;
  free_func_t free_func;
} flist_t;

typedef uint8_t bool_t;


string_t *string_new(char *ptr, size_t len)
{
  string_t *str = malloc(sizeof(string_t));
  if (str) {
    if (ptr && len) {
      str->ptr = ptr;
      str->len = strlen(ptr);
    } else {
      str->ptr = NULL;
      str->len = 0;
    }
  }
  return str;
}

void string_free(string_t *str, bool_t free_ptr)
{
  if (!str) return;
  if (free_ptr) {
    if (str->ptr) free(str->ptr);
  }
  free(str);
}

flist_t *flist_new(void *ptr, size_t len)
{
  flist_t *list = malloc(sizeof(flist_t));
  if (list) {
    if (ptr && len) {
      list->ptr = ptr;
      list->len = len;
    } else {
      list->ptr = NULL;
      list->len = 0;
    }
    list->free_func = free;
  }

  return list;
}

void flist_set_free_func(flist_t *list, free_func_t func)
{
  if (!list) return;
  list->free_func = func;
}

void flist_clean(flist_t *list)
{
  if (!list) return;
  if (list->ptr) list->free_func(list->ptr);
  list->ptr = NULL;
  list->len = 0;
}

void flist_free(flist_t *list, bool_t free_ptr)
{
  if (!list) return;
  if (free_ptr) {
    flist_clean(list);
  }
  free(list);
}





/*
 * this function doesn't allocate buffer to copy tokens
 * n: return n tokens (0 means all tokens)
 * len: is value return argument, it contains the string length
 * also gets updated by the function to say how many tokens were found
 * caller must free the token list when done
 */
flist_t *str_tokens(const char *delim, string_t *str, size_t n)
{
  flist_t *tlist = NULL;
  size_t cnt = 0;
  token_t *t = NULL;
  token_t *tmp = NULL;
  size_t tmp_len = 0;
  size_t tmp_step = 0;
  size_t ntok = 0;
  uint8_t keep_going = 1;
  size_t tlen = 0;
  char *tdelim = NULL;
  char *d = NULL;
  char *tstr = NULL;
  uint8_t split_type = 0;
  uint8_t take = 0;
  uint8_t eat_next_empty = 0;
#if 0
  uint8_t last_tok_was_empty = 0;
#endif

  pcre *re = NULL;
  const char *pcre_err_msg = NULL;
  int pcre_err = 0;
  int rc = 0;
#define MAX_OFFSETS 10
  int offsets[MAX_OFFSETS*3];
  size_t step = 0;
  size_t slen = 0;
  char *pstr = NULL;


  tlist = flist_new(NULL, 0);
  if (!tlist) return NULL;

#define DEFAULT_TOKEN_NUM 32
#define DEFAULT_TOKEN_STEP 8
  if (!str || !str->len) goto end;
  if (!delim) goto end;




  if (n == 0) ntok = DEFAULT_TOKEN_NUM;
  else ntok = n;
  step = DEFAULT_TOKEN_STEP;

  t = malloc(ntok * sizeof(token_t));
  if (!t) goto end;

  pstr = str->ptr;


#define DELIM_CHAR_EVERY 1
#define DELIM_CHAR_CLASS 2
#define DELIM_CHAR_REGEX 3
#define DELIM_CHAR_ARRAY 4

  tdelim = strdup(delim); /* we will work with our own copy */
  if (!tdelim) goto end;
  d = tdelim;
  tlen = strlen(d);
  if (*d == '\0') {
    split_type = DELIM_CHAR_EVERY;
  } else if (*d == '[') { /* [xyz] or [xyz]+ */
    ++d;
    --tlen;
    split_type = DELIM_CHAR_CLASS;
    if (*(d+tlen-1) == '+') {
      eat_next_empty = 1;
      *(d+tlen-1) = '\0';
      --tlen;
    }
    if (*(d+tlen-1) == ']') {
      *(d+tlen-1) = '\0'; /* eat ending ] */
      --tlen;
    }
  } else if (*d == '/') { /* regex */
    slen = str->len;
    ++d;
    --tlen;
    split_type = DELIM_CHAR_REGEX;
    if (*(d+tlen-1) == '/') {
      *(d+tlen-1) = '\0'; /* eat ending / */
      --tlen;
    }

    re = pcre_compile(d, 0, &pcre_err_msg, &pcre_err, NULL);
    if (re == NULL) {
      fprintf(stderr, "Error compiling regex\n");
      goto end;
    }
  } else {
    split_type = DELIM_CHAR_ARRAY; /* string */
    slen = str->len;
  }

  tmp = t;
  while (keep_going) {
    if (cnt >= ntok) {
      ntok += step;
      tmp = realloc(t, ntok * sizeof(token_t));
      if (!tmp) { /* realloc failed, original t is untouched */
	goto end;
      }
      t = tmp; /* newly extended memory */
      tmp = t + cnt; /* point to the last location worked */
    }

    take = 0;
    tmp_len = 0;
    tmp_step = 0;
    switch (split_type) {
      case DELIM_CHAR_EVERY:
	tmp_len = 1;
	tmp_step = 1;
	break;
      case DELIM_CHAR_CLASS:
	tmp_len = strcspn(pstr, d);
	tmp_step = tmp_len;
	if (*(pstr+tmp_len) != '\0') ++tmp_step;
	break;
      case DELIM_CHAR_REGEX:
	rc = pcre_exec(re, NULL, pstr, slen, 0, 0, offsets, MAX_OFFSETS*3);
	if (rc <= 0) goto end; /* no match or error */
	tmp_len = offsets[0];
	tmp_step = offsets[1];
	break;
      case DELIM_CHAR_ARRAY:
	tstr = strstr(pstr, d);
	if (tstr) {
	  tmp_len = tstr - pstr;
	  tmp_step = tmp_len;
	  if (*(pstr+tmp_len) != '\0') ++tmp_step;
	} else {
	  tmp_len = slen;
	  tmp_step = tmp_len;
	}
	break;
      default:
	keep_going = 0;
	break;
    }

    if (tmp_len == 0) {
      if (!eat_next_empty) {
	take = 1;
      }
#if 0
      last_tok_was_empty = 1;
#endif
    } else {
      take = 1;
#if 0
      last_tok_was_empty = 0;
#endif
    }
    if (take) {
      tmp->ptr = pstr;
      tmp->len = tmp_len;
      ++tmp;
      ++cnt;
    }
    pstr += tmp_step;
    slen -= tmp_step;

    if (n && cnt >= n) keep_going = 0;
    else if (*pstr == '\0') keep_going = 0;
  }


end:
  if (tdelim) free(tdelim);
  if (re) pcre_free(re);
  tlist->ptr = t;
  tlist->len = cnt;
  return tlist;
}

string_t *file_get_contents(const char *file)
{
  string_t *str = NULL;
  char *buf = NULL;
  ssize_t n = 0;
  struct stat st;
  memset(&st, 0, sizeof(st));
  stat(file, &st);
  buf = malloc(st.st_size+1);
  int fd = 0;
  
  fd = open(file, O_RDONLY);
  if (fd >= 0) {
    n = read(fd, buf, st.st_size);
    close(fd);
  }
  if (n != st.st_size) {
    fprintf(stderr, "Less data read, %ld bytes.\n", n);
  }
  buf[n] = '\0';
  str = string_new(buf, n);
  if (!str) {
    free(buf); /* can't proceed with memory */
  }
  return str;
}

void file_put_contents(const char *file, string_t *str)
{
  ssize_t n = 0;
  int fd = open(file, O_CREAT|O_WRONLY, 0644);
  if (fd >= 0) {
    n = write(fd, str->ptr, str->len);
    if (n != str->len) {
      fprintf(stderr, "Not all data written, attempted %lu, wrote %ld bytes.\n", str->len, n);
    } else {
      fprintf(stderr, "Written %ld bytes of data\n", n);
    }
    close(fd);
  }
}

/* caller should not free unless nreplaced > 0 */
string_t *str_replace(const char *search, const char *replace, string_t *haystack, size_t *nreplaced)
{
  size_t len = 0;
  size_t rlen = strlen(replace);
  char *pstr = NULL;
  size_t n = 0; /* replace all occurrences */
  int i = 0;
  size_t flen = 0;
  char *final = NULL;
  token_t *toks = NULL;
  string_t *str_final = NULL;

  *nreplaced = 0;
  flist_t *tlist = str_tokens(search, haystack, n);
  if (tlist) {
    toks = (token_t *) tlist->ptr;
    len = tlist->len;
    fprintf(stderr, "Total %lu tokens found for replacement\n", len);
    flen = 0;
    for (i = 0; i < len; ++i) {
      flen += toks[i].len + rlen;
    }
    ++flen; /* for terminating NUL byte */
    final = malloc(flen);
    if (!final) goto end;

    fprintf(stderr, "Total %lu bytes allocated for str_replace\n", flen);

    *nreplaced = len;
    pstr = final;
    for (i = 0; i < len; ++i) {
      memcpy(pstr, toks[i].ptr, toks[i].len);
      pstr += toks[i].len;
      memcpy(pstr, replace, rlen);
      pstr += rlen;
    }
    final[flen-1] = '\0';
    flist_free(tlist, 1);
  } else {
    final = (char *) haystack->ptr;
    flen = haystack->len;
  }

end:
  str_final = string_new(final, flen);
  return str_final;
}

int main(int argc, char *argv[])
{
  size_t n = 0;
  size_t nrepl = 0;
  string_t *final = NULL;
  char *delim = "[\r\n]+"; /* split list of given chars, no empty */
#if 0
  char *delim = "[\r\n]"; /* use chars */
  char *delim = "\r\n";
  char *delim = "/[\r\n]+/"; /* use regex */
  char *delim = ""; /* split every char */
  char *str = "hi|there, this is ayub\n";
#endif
  flist_t *tlist = NULL;
  string_t *str = NULL;
  token_t *toks = NULL;
  int i = 0;

  str = file_get_contents("cad_yes.csv");
  if (str && str->len) {
    fprintf(stderr, "File read with contents of %lu bytes\n", str->len);
    tlist = str_tokens(delim, str, n);
    if (tlist) {
      fprintf(stderr, "Total %lu tokens found\n", tlist->len);
    }

    /*
    toks = (token_t *) tlist->ptr;
    if (toks && tlist->len > 0) {
      for (i = 0; i < tlist->len; ++i) {
	  fprintf(stdout, "record %08d: %lu\n", i, toks[i].len);
      }
    }
    */

    flist_free(tlist, 1);

    tlist = str_tokens(delim, str, n);
    if (tlist) {
      fprintf(stderr, "Total %lu tokens found\n", tlist->len);
    }
    flist_free(tlist, 1);
  }

  final = str_replace(delim, "\n", str, &nrepl);
  if (final && nrepl > 0) {
    fprintf(stderr, "Total %lu patterns replaced\n", nrepl);
    file_put_contents("cad_yes_unix.csv", final);
    string_free(final, 1);
  }

  string_free(str, 1);


  return 0;
}
