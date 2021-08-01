/*
 * assoc_array_impl.h
 */

#ifndef _ASSOC_ARRAY_IMPL_H_
#define _ASSOC_ARRAY_IMPL_H_

#include "config.h"

/*
 * array_t, array_iter_t are defined in the implementation
 */
#ifdef USE_ASSOC_ARRAY_IMPL_HATTRIE
#include "assoc_array_impl_hattrie.h"
#else
#error "At least one implementation has to be used for assoc array"
#endif

#endif
