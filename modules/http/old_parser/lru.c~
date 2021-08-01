/*
 * lru.c
 * http://jehiah.cz/a/uthash
 */
#include <stdio.h>
#include <string.h>
#include <uthash.h>

// this is an example of how to do a LRU cache in C using uthash
// http://uthash.sourceforge.net/
// by Jehiah Czebotar 2011 - jehiah@gmail.com
// this code is in the public domain http://unlicense.org/

#define MAX_CACHE_SIZE 10000000

struct CacheEntry {
  char *key;
  char *value;
  UT_hash_handle hh;
};
static struct CacheEntry *cache;

char *lru_find_in_cache(const char *key)
{
  struct CacheEntry *entry;
  HASH_FIND_STR(cache, key, entry);
  if (entry) {
    //fprintf(stderr, "Found\n");
    // remove it (so the subsequent add will throw it on the front of the list)
    HASH_DELETE(hh, cache, entry);
    HASH_ADD_KEYPTR(hh, cache, entry->key, strlen(entry->key), entry);
    return entry->value;
  }
  return NULL;
}

void lru_add_to_cache(const char *key, const char *value)
{
  struct CacheEntry *entry, *tmp_entry;
  entry = malloc(sizeof(struct CacheEntry));
  entry->key = strdup(key);
  entry->value = strdup(value);
  HASH_ADD_KEYPTR(hh, cache, entry->key, strlen(entry->key), entry);

  // prune the cache to MAX_CACHE_SIZE
  if (HASH_COUNT(cache) >= MAX_CACHE_SIZE) {
    HASH_ITER(hh, cache, entry, tmp_entry) {
      // prune the first entry (loop is based on insertion order so this deletes the oldest item)
      HASH_DELETE(hh, cache, entry);
      free(entry->key);
      free(entry->value);
      free(entry);
      break;
    }
  }    
}
