#include <stdio.h>
#include <string.h>
#include "persistence.h"
#include "parser.h"

#define AOF_FILENAME "mini_redis.aof"

void aof_append(const char *command_line) {
    FILE *file = fopen(AOF_FILENAME, "a");
    if (!file) {
        perror("Failed to open AOF file for writing");
        return;
    }
    fprintf(file, "%s\n", command_line);
    fclose(file);
}

void aof_load(HashTable *ht) {
    FILE *file = fopen(AOF_FILENAME, "r");
    if (!file) {
        return;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = '\0';
        Command cmd = parse_command(line);
        if (cmd.type == CMD_SET) {
            ht_set(ht, cmd.key, cmd.value);
        }
        else if (cmd.type == CMD_DEL)
            ht_delete(ht, cmd.key);
    }
    fclose(file);
}