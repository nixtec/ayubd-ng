#ifdef USE_PROG

#ifndef _PROG_H_
#define _PROG_H_

/* Make sure this header file is available.*/
#ifdef USE_PH7
#include "lib/PH7/ph7.h"

typedef struct {
  ph7 *pEngine;
  ph7_vm *pVm;
} prog_t;

#endif

#ifdef USE_UNQLITE
#include "lib/unqlite/unqlite.h"

typedef struct {
  unqlite *pEngine; /* Actually DB */
  unqlite_vm *pVm;
} prog_t;

#endif

typedef struct {
  char *str;
  size_t len;
  size_t alen; /* allocated length */
} prog_out_t;

/* usually raw http header packet will be sent using this */
typedef struct {
  const char *str;
  int len;
} prog_in_t;

prog_in_t *prog_in_new(size_t len);
void prog_in_free(prog_in_t *pi);
prog_out_t *prog_out_new(size_t len);
void prog_out_free(prog_out_t *po);

#define prog_in_reset(pi) ((pi)->len=0)
#define prog_out_reset(po) ((po)->len=0)

prog_t *prog_init(const char *file);
void prog_exec(prog_t *prog, prog_in_t *pi, prog_out_t *po);
void prog_done(prog_t *prog);

#endif

#endif /* !USE_PROG */
