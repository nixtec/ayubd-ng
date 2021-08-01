/*
 * tokenize.c
 */
#include "tokenize.h"

/*
typedef struct {
  int s;
  int l;
} tinfo;
*/

/*
 * delim: any of the char in delim will be used to tokenize
 * str: null terminated string
 * len: will be filled up how many tokens are there
 * nmax: maximum n (in cas of n=-1 all) tokens to be returned
 * caller must 'free' the allocated memory
 */
token_t *explode_n(const char *delim, char *str, size_t *len, size_t nmax)
{
#define DEFAULT_TOKEN_NUM 32
#define DEFAULT_TOKEN_STEP 8
  if (!buf) return NULL;
  char *pstr = str;
  size_t cnt = 0;
  char *tok = NULL;
  token_t *t = NULL;
  token_t *tmp = NULL;
  size_t ntok = 0;
  if (nmax < 0) {
    ntok = DEFAULT_TOKEN_NUM;
  } else if (nmax > 0) {
    ntok = n;
  } else { /* nmax == 0 */
    return NULL;
  }

  t = malloc(DEFAULT_TOKEN_NUM * sizeof(token_t));
  tmp = t;
  while (pstr) {
    if (cnt > ntok) {
      ntok += DEFAULT_TOKEN_STEP * sizeof(token_t);
      t = realloc(t, ntok);
      tmp = t+cnt; /* realloc may return different address than before */
    }
    tmp->ptr = strsep(&pstr, delim);
    tmp->len = pstr - tmp->ptr - 1; /* -1 to skip the NUL byte after token */
    ++cnt;
    ++tmp;
    if (nmax > 0 && cnt > nmax) break;
  }

  *len = cnt;

  return t;
}
