#include <stdlib.h>
#include <string.h>
#include "hashtable.h"  

static unsigned long hash_function(const char *key) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++)) {
        hash = (hash << 5) + hash + c;
    }
    return hash;
}

HashTable *ht_create(size_t capacity) {
    HashTable *ht = malloc(sizeof(HashTable));
    if (!ht)
        return NULL;
    ht->buckets = calloc(capacity, sizeof(Entry *));
    if (!ht->buckets) {
        free(ht);
        return NULL;
    }
    ht->capacity = capacity;
    ht->size = 0;
    return ht;
}

void ht_destroy(HashTable *ht) {
    if (!ht)
        return;
    for (size_t i = 0; i < ht->capacity; i++) {
        Entry *entry = ht->buckets[i];
        while (entry) {
            Entry *next = entry->next;
            free(entry->key);
            free(entry->value);
            free(entry);
            entry = next;
        }
    }
    free(ht->buckets);
    free(ht);
}

void ht_set(HashTable *ht, const char *key, const char *value) {
    unsigned long index = hash_function(key) % ht->capacity;

    Entry *entry = ht->buckets[index];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            free(entry->value);
            entry->value = strdup(value);
            return;
        }
        entry = entry->next;
    } 
    Entry *new_entry = malloc(sizeof(Entry));
    new_entry->key = strdup(key);
    new_entry->value = strdup(value);
    new_entry->next = ht->buckets[index];
    ht->buckets[index] = new_entry;
    ht->size++; 
}

const char *ht_get(HashTable *ht, const char *key) {
    unsigned long index = hash_function(key) % ht->capacity;
    Entry *entry = ht->buckets[index];

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL;
}

int ht_delete(HashTable *ht, const char *key) {
    unsigned long index = hash_function(key) % ht->capacity;
    Entry *entry = ht->buckets[index];
    Entry *prev = NULL;
    
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            if (prev == NULL)   
                ht->buckets[index] = entry->next;
            else 
                prev->next = entry->next;
            free(entry->key);
            free(entry->value);
            free(entry);
            ht->size--;
            return 1;
        }
        prev = entry;
        entry = entry->next;
    }
    return 0;
}