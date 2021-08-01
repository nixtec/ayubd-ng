/*
 * assoc_array.h
 */
#ifndef _ASSOC_ARRAY_H_
#define _ASSOC_ARRAY_H_


#include <stdio.h>
#include <stdarg.h>
#include "assoc_array_impl.h"

/*
 * following macro is just for the ease of use
 * it uses strlen(), so don't use it in loops or where same length calculation is executed several times
 */
#define _aget(a,k) array_get(a,k,strlen(k)+1)
#define _aset(a,k,v) array_set(a,k,strlen(k)+1,(value_t)v)
/*
 * _ahaskey: checks if a given key exists (uses hattrie_tryget)
 */
#define _ahaskey(a,k) array_key_exists(a,k,strlen(k)+1)

/* arg is user supplied location where function can save states or do something interesting */
typedef void (*array_walk_fn_t)(char *key, size_t klen, char *val, void *arg);

void array_walk_impl(array_t *a, array_walk_fn_t fn, uint8_t is_ord);
#define array_walk(a,fn)		array_walk_impl((a), (fn), 0)
#define array_walk_ord(a,fn)		array_walk_impl((a), (fn), 1)

void array_free_values(array_t *a);

#ifndef ARRAY_KEY_MAX
#define ARRAY_KEY_MAX 512
#endif

/* SUBSEP as in AWK (use %c in varargs) */
#define SUBSEP 0x1c

/* add NULL at the end of argument index to make sure it ends there */
/* all key arguments are of type 'char *' */
size_t array_key_make(char *key, size_t len, const char *arg1, ...);



#endif
