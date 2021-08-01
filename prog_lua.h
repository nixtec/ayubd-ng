#ifndef _PROG_H_
#define _PROG_H_

/* Make sure this header file is available.*/
#include "ph7.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct {
  lua_State *L;
} prog_t;

typedef struct {
  char *str;
  size_t len;
  size_t alen; /* allocated length */
} prog_out_t;

prog_out_t *prog_out_new(size_t len);
void prog_out_free(prog_out_t *po);

#define prog_out_reset(po) ((po)->len=0)

prog_t *prog_init(const char *file);
void prog_exec(prog_t *prog, prog_out_t *po);
void prog_done(prog_t *prog);

#endif
