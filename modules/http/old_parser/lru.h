/*
 * lru.h
 */
#ifndef __LRU_H__
#define __LRU_H__

char *lru_find_in_cache(const char *key);

void lru_add_to_cache(const char *key, const char *val);


#endif
