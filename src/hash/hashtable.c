#include "hashtable.h"

static int hash_round_size(int size) {
  int i = 1;
  while (i < size) {
    i <<= 1;
  }
  return i;
}

static bool default_cmp_func(const char *key1, const char *key2, size_t len) {
  return !strncmp(key1, key2, len);
}

static uint64_t fnv_hash(char *key, int n) {
  uint64_t hash = 0xcbf29ce484222325;
  for (int i = 0; i < n; ++i) {
    hash *= 0x100000001b3;
    hash ^= key[i];
  }
  return hash;
}

int htable_init(struct hashtable *ht, hashtable_cmp_func cmp_func) {
  ht->buckets = calloc(1, HASHTABLE_INITSIZE * sizeof(struct hashtable_entry *));
  if (!ht->buckets) {
    return 1;
  }
  ht->nentries = 0;
  ht->nbuckets = HASHTABLE_INITSIZE;
  if (!cmp_func) {
    ht->cmp_func = default_cmp_func;
  } else {
    ht->cmp_func = cmp_func;
  }
  return 0;
}

int htable_push(struct hashtable *ht, char *key, int len, void *value) {
  uint64_t hash = fnv_hash(key, len);
  struct hashtable_entry *new_entry = malloc(sizeof(struct hashtable_entry));
  if (!new_entry) {
    return 1;
  }
  ++ht->nentries;
  if (ht->nentries / ht->nbuckets > HASHTABLE_HIGH) {
    if (htable_rehash(ht, ht->nbuckets * HASHTABLE_GROWTH_FACTOR)) {
      return 1;
    }
  }
  new_entry->key = key;
  new_entry->hash_key = hash;
  new_entry->len = len;
  new_entry->value = value;
  new_entry->next = ht->buckets[hash & (ht->nbuckets - 1)];
  ht->buckets[hash & (ht->nbuckets - 1)] = new_entry;
  return 0;
}

int htable_rehash(struct hashtable *ht, int new_size) {
  new_size = hash_round_size(new_size);
  struct hashtable_entry **new_buckets = calloc(1, sizeof(struct hashtable_entry *) * new_size);
  if (!new_buckets) {
    return 1;
  }
  for (int i = 0; i < ht->nbuckets; ++i) {
    struct hashtable_entry *entry = ht->buckets[i];
    while (entry) {
      new_buckets[entry->hash_key & (new_size - 1)] = entry;
      entry = entry->next;
    }
  }
  free(ht->buckets);
  ht->buckets = new_buckets;
  ht->nbuckets = new_size;
  return 0;
}

void *htable_get(struct hashtable *ht, char *key, int len) {
  uint64_t hash = fnv_hash(key, len);
  struct hashtable_entry *entry = ht->buckets[hash & (ht->nbuckets - 1)];
  while (entry) {
    if (hash == entry->hash_key && ht->cmp_func(key, entry->key, len)) {
      return entry->value;
    }
    entry = entry->next;
  }
  return NULL;
}

bool htable_contains(struct hashtable *ht, char *key, int len) {
  uint64_t hash = fnv_hash(key, len);
  struct hashtable_entry *entry = ht->buckets[hash & (ht->nbuckets - 1)];
  while (entry) {
    if (hash == entry->hash_key && ht->cmp_func(key, entry->key, len)) {
      return 1;
    }
    entry = entry->next;
  }
  return 0;
}

void htable_remove(struct hashtable *ht, char *key, int len) {
  uint64_t hash = fnv_hash(key, len);
  struct hashtable_entry *entry = ht->buckets[hash & (ht->nbuckets - 1)];
  struct hashtable_entry *prev_entry = NULL;
  while (entry) {
    if (hash == entry->hash_key && ht->cmp_func(key, entry->key, len)) {
      if (!prev_entry) {
        ht->buckets[hash & (ht->nbuckets - 1)] = entry->next;
      } else {
        prev_entry->next = entry->next;
      }
      if (entry->value) {
        free(entry->value);
      }
      --ht->nentries;
      free(entry);
    }
    prev_entry = entry;
    entry = entry->next;
  }
}

