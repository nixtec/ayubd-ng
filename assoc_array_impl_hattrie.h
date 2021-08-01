/*
 * assoc_array_impl_hattrie.h
 * Implementation of Associative Array using HAT-trie (https://github.com/dcjones/hat-trie)
 */
#ifndef _ASSOC_ARRAY_IMPL_HATTRIE_H_
#define _ASSOC_ARRAY_IMPL_HATTRIE_H_


#include <hat-trie/hat-trie.h>


typedef hattrie_t array_t;
typedef hattrie_iter_t array_iter_t;

#define array_new()			hattrie_create()
#define array_free(a)			hattrie_free((a))
#define array_clear(a)			hattrie_clear((a))
#define array_size(a)			hattrie_size((a))
#define array_meminfo(a)		hattrie_sizeof((a))
/*
 * #define array_get(a,k,klen)		hattrie_tryget((a), (k), (klen))
 */
/* awk style get, which will create blank entry if not exists */
#define array_key_exists(a,k,klen)	(hattrie_tryget((a), (k), (klen)) == NULL? 0 : 1)
#define array_get(a,k,klen)		*hattrie_get((a), (k), (klen))
#define array_set(a,k,klen,val)		do { *(hattrie_get((a), (k), (klen))) = (val); } while (0)
#define array_del(a,k,klen)		hattrie_del((a), (k), (klen))
/* unordered iteration */
#define array_iter_new(a)		hattrie_iter_begin((a), 0)
/* ordered iteration */
#define array_iter_new_ord(a)		hattrie_iter_begin((a), 1)
#define array_iter_next(iter)		hattrie_iter_next((iter))
#define array_iter_key(iter,plen)	hattrie_iter_key((iter), (plen))
#define array_iter_val(iter)		hattrie_iter_val((iter))
#define array_iter_finished(iter)	hattrie_iter_finished((iter))
#define array_iter_free(iter)		hattrie_iter_free((iter))

#define array_iter_equal(iter1,iter2)	hattrie_iter_equal((iter1), (iter2))

/* since trie doesn't provide a dup routine, we are implementing it using iterator */
#define array_dup(a,b)			do {								\
						array_iter_t *iter = array_iter_new(a);			\
						(b) = array_new();					\
						const char *ikey;					\
						size_t ilen;						\
						while (!array_iter_finished(iter)) {			\
							ikey = array_iter_key(iter, &ilen);		\
							array_set((b),ikey,ilen,*array_iter_val(iter));	\
							array_iter_next(iter);				\
						}							\
						array_iter_free(iter);					\
					} while (0)
					

#endif
