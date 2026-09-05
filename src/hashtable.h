#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stddef.h>

typedef struct Entry {
    char *key;
    char *value;
    struct Entry *next;
} Entry;

typedef struct {
    Entry **buckets;
    size_t capacity;
    size_t size;
} HashTable;

HashTable *ht_create(size_t capacity);
void ht_destroy(HashTable *ht);
void ht_set(HashTable *ht, const char *key, const char *value);
const char *ht_get(HashTable *ht, const char *key);
int ht_delete(HashTable *ht, const char *key);
#endif // !HASHTABLE_H

