#include <stdio.h>
#include "hashtable.h"

int main(void) {
    HashTable *ht = ht_create(16);

    ht_set(ht, "language", "Python");
    ht_set(ht, "framework", "FastAPI");
    ht_set(ht, "database", "PostgreSQL");

    printf("language: %s\n", ht_get(ht, "language"));
    printf("framework: %s\n", ht_get(ht, "framework"));
    printf("database: %s\n", ht_get(ht, "database"));

    const char *missing = ht_get(ht, "does_not_exist");
    printf("missing key: %s\n", missing ? missing : "NULL");

    ht_set(ht, "language", "Rust");
    printf("after update, language: %s\n", ht_get(ht, "language"));

    int deleted  = ht_delete(ht, "framework");
    printf("delete 'framework' succeeded: %d\n", deleted);
    printf("framework after delete: %s\n", ht_get(ht, "framework"));

    printf("total keys remaining: %zu\n", ht->size);

    ht_destroy(ht);
    return 0;
}