#ifndef _HASHMAP_H
#define _HASHMAP_H

#include <stdint.h>

#define HASHTABLE_LOW 0.1
#define HASHTABLE_HIGH 0.5
#define HASHTABLE_INITSIZE 2
#define HASHTABLE_GROWTH_FACTOR 2

typedef int (*hashtable_cmp_func)(const char *key1, const char *key2, size_t len);

struct hashtable_entry {
  struct hashtable_entry *next;
  void *value;
  char *key;
  int len;
  uint64_t hash_key;
};

struct hashtable {
  struct hashtable_entry **buckets;
  int nentries;
  int nbuckets;
  hashtable_cmp_func cmp_func;
};

int htable_init(struct hashtable *ht, hashtable_cmp_func cmp_func);
int htable_push(struct hashtable *ht, char *key, int len, void *value);
void *htable_get(struct hashtable *ht, char *key, int len);
int htable_rehash(struct hashtable *h, int new_sizet);
void htable_remove(struct hashtable *ht, char *key, int len);

#endif // _HASHMAP_H

