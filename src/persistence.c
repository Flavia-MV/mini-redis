#include <stdio.h>
#include <string.h>
#include "persistence.h"
#include "parser.h"

#define AOF_FILENAME "mini_redis.aof"
static FILE *aof_file = NULL;

void aof_open(void) {
    aof_file = fopen(AOF_FILENAME, "a");
    if (!aof_file)
        perror("Failed to open AOF file");
}

void aof_close(void) {
    if (aof_file != NULL) {
        fclose(aof_file);
        aof_file = NULL;
    }
}

void aof_append(const char *command_line) {
    if (!aof_file) {
        return;
    }
    fprintf(aof_file, "%s\n", command_line);
    fflush(aof_file);
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