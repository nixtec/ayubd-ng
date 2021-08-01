/*
 * config.h
 */
#ifndef __CONFIG_H__
#define __CONFIG_H__

/* use associative array implementation using HAT-trie */
#define USE_ASSOC_ARRAY_IMPL_HATTRIE 1

#ifdef USE_TCMALLOC
/* TCMALLOC */
/* TCMALLOC will be loaded with LD_PRELOAD command. Because liblfds and others
 * will allocate using system's malloc. So, to use tcmalloc for all I will use
 * LD_PRELOAD method.
 * Also to consume less memory I will use the libtcmalloc_minimal.so and strip
 * it.
 */
#include <google/tcmalloc.h>
#define DO_MALLOC tc_malloc
#define DO_CALLOC tc_calloc
#define DO_RELLOC tc_realloc
#define DO_FREE tc_free
#else
#include <stdlib.h>
#define DO_MALLOC malloc
#define DO_CALLOC calloc
#define DO_REALLOC realloc
#define DO_FREE free
#endif

#define IOV_CHUNK 4096

#endif
