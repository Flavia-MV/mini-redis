#include <stdlib.h>
#include <string.h>
#include "hashtable.h"  

#define HT_MAX_LOAD_FACTOR 0.75

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

static int ht_resize(HashTable *ht) {
    size_t new_capacity = ht->capacity * 2;
    Entry **new_buckets = calloc(new_capacity, sizeof(Entry*));
    if (!new_buckets)
        return 0;
    
    for (size_t i = 0; i < ht->capacity; i++) {
        Entry *entry = ht->buckets[i];
        while (entry) {
            Entry *next = entry->next;
            unsigned long index = hash_function(entry->key) % new_capacity;
            entry->next = new_buckets[index];
            new_buckets[index] = entry;
            entry = next;
        }
    }
    free(ht->buckets);
    ht->buckets = new_buckets;
    ht->capacity = new_capacity;
    return 1;
}

int ht_set(HashTable *ht, const char *key, const char *value) {
    unsigned long index = hash_function(key) % ht->capacity;

    Entry *entry = ht->buckets[index];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            char *new_value = strdup(value);
            if (!new_value)
                return 0;
            free(entry->value);
            entry->value = new_value;
            return 1;
        }
        entry = entry->next;
    } 

    if ((double)(ht->size + 1) / (double)ht->capacity > HT_MAX_LOAD_FACTOR) {
        if (ht_resize(ht))
            index = hash_function(key) % ht->capacity;
    } 
    Entry *new_entry = malloc(sizeof(Entry));
    if (!new_entry)
        return 0;
    new_entry->key = strdup(key);
    new_entry->value = strdup(value);
    if (!new_entry->key || !new_entry->value) {
        free(new_entry->key);
        free(new_entry->value);
        free(new_entry);
        return 0;
    }
    new_entry->next = ht->buckets[index];
    ht->buckets[index] = new_entry;
    ht->size++; 
    return 1;
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