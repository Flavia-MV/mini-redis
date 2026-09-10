#include <string.h>
#include "parser.h"

static int safe_copy(char *dst, size_t dst_size, const char *src) {
    size_t len = strlen(src);
    if (len >= dst_size)
        return 0;
    memcpy(dst, src, len+1);
    return 1;
}

Command parse_command(const char *input) {
   
    Command cmd;
    cmd.key[0] = '\0';
    cmd.value[0] = '\0';
    size_t input_len = strlen(input);
    if (input_len >= PARSER_MAX_LINE) {
        cmd.type = CMD_TOO_LONG;
        return cmd;
    }

    char buffer[PARSER_MAX_LINE];
    memcpy(buffer, input, input_len + 1);
    char *first_word = strtok(buffer, " ");

    if (first_word == NULL) {
        cmd.type = CMD_UNKNOWN;
        return cmd;
    }

    if (strcmp(first_word, "SET") == 0)   
        cmd.type = CMD_SET;
    else if (strcmp(first_word, "GET") == 0)
        cmd.type = CMD_GET;
        else if (strcmp(first_word, "DEL") == 0)
            cmd.type = CMD_DEL;
            else
                cmd.type = CMD_UNKNOWN;

    char *second_word = strtok(NULL, " ");
    if (second_word == NULL) {
        return cmd;
    }

    if (!safe_copy(cmd.key, sizeof(cmd.key), second_word)) {
        cmd.type = CMD_TOO_LONG;
        return cmd;
    }

    if (cmd.type == CMD_SET) {
        char *third_word = strtok(NULL, " ");
        if (third_word != NULL)
            if (!safe_copy(cmd.value, sizeof(cmd.value), third_word)) {
                cmd.type = CMD_TOO_LONG;
                return cmd;
            }
        
    } 
    return cmd;
}

