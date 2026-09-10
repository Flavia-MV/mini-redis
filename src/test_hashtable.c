#include <stdio.h>
#include <string.h>
#include "hashtable.h"

static int failures = 0;

#define CHECK(cond, description) do { \
    if (!(cond)) { \
        printf("FAIL: %s\n", description); \
        failures++; \
    } else { \
        printf("PASS: %s\n", description); \
    } \
} while(0)

static void test_set_and_get(void) {
    HashTable *ht = ht_create(4);
    CHECK(ht_set(ht, "name", "Anne"), "ht_set returns success");
    const char *v = ht_get(ht, "name");
    CHECK(v != NULL && strcmp(v, "Anne")== 0, "ht_get returns the value that was set");
    ht_destroy(ht);
}

static void test_get_missing_key(void) {
    HashTable *ht = ht_create(4);
    CHECK(ht_get(ht, "missing") == NULL, "ht_get returns NULL for a missing key");
    ht_destroy(ht);
}

static void test_overwrite_existing_key(void) {
    HashTable *ht = ht_create(4);
    ht_set(ht, "key", "first");
    ht_set(ht, "key", "second");
    const char *v = ht_get(ht, "key");
    CHECK(v != NULL && strcmp(v, "second") == 0, "setting an existing key overwrites its value");
    CHECK(ht->size == 1, "overwriting a key does not increase size");
    ht_destroy(ht);
}

static void test_delete(void) {
    HashTable *ht = ht_create(4);
    ht_set(ht, "key", "value");
    CHECK(ht_delete(ht, "key") == 1, "ht_delete returns 1 for an existing key");
    CHECK(ht_get(ht, "key") == NULL, "deleted key is no longer retrievable");
    CHECK(ht_delete(ht, "key") == 0, "deleting an already-deleted key return 0");
    ht_destroy(ht);
}

static void test_delete_missing_key(void) {
    HashTable *ht = ht_create(4);
    CHECK(ht_delete(ht, "nope") == 0, "ht_delete on a missing key returns 0");
    ht_destroy(ht);
}

static void test_collision_chaining(void) {
    HashTable *ht = ht_create(1);
    ht_set(ht, "a", "1");
    ht_set(ht, "b", "2");
    ht_set(ht, "c", "3");
    CHECK(strcmp(ht_get(ht, "a"), "1") == 0, "colliding key 'a' still retrievable");
    CHECK(strcmp(ht_get(ht, "b"), "2") == 0, "colliding key 'b' still retrievable");
    CHECK(strcmp(ht_get(ht, "c"), "3") == 0, "colliding key 'c' still retrievable");
    ht_delete(ht, "b");
    CHECK(strcmp(ht_get(ht, "a"), "1") == 0, "deleting middle-of-chain entry preserves 'a'");
    CHECK(strcmp(ht_get(ht, "c"), "3") == 0, "deleting middle-of-chain entry preserves 'c'");
    CHECK(ht_get(ht, "b") == NULL, "deleted middle-of-chain entry 'b' is gone");
    ht_destroy(ht);   
}

static void test_resize_on_growth(void) {
    HashTable *ht = ht_create(4);
    size_t initial_capacity = ht->capacity;
    
    char key[32];
    for (int i = 0; i < 100; i++) {
        snprintf(key, sizeof(key), "key%d", i);
            ht_set(ht, key, "v");
    }
    CHECK(ht->capacity > initial_capacity, "table capacity grows as entries are added");
    CHECK(ht->size == 100, "size correctly reflects all inserted entries after resize");

    int all_found = 1;
    for (int i = 0; i < 100; i++) {
        snprintf(key, sizeof(key), "key%d", i);
        const char *v = ht_get(ht, key);
        if (!v || strcmp(v, "v") != 0) {
            all_found = 0;
            break;
        }
    }
    CHECK(all_found, "all entries remain retrievable after a resize");
    ht_destroy(ht);
}

int main(void) {
    test_set_and_get();
    test_get_missing_key();
    test_overwrite_existing_key();
    test_delete();
    test_delete_missing_key();
    test_collision_chaining();
    test_resize_on_growth();

    printf("\n%d failures\n", failures);
    return failures == 0 ? 0 : 1;
}