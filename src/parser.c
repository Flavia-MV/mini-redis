#include <string.h>
#include "parser.h"

Command parse_command(const char *input) {
    char buffer[512];
    strcpy(buffer, input);
    char *first_word = strtok(buffer, " ");

    Command cmd;
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
        cmd.key[0] = '\0';
        cmd.value[0] = '\0';
        return cmd;
    }

    strcpy(cmd.key, second_word);

    if (cmd.type == CMD_SET) {
        char *third_word = strtok(NULL, " ");
        if (third_word != NULL)
            strcpy(cmd.value, third_word);
        else
            cmd.value[0] = '\0';
    } else {
        cmd.value[0] = '\0';
    }
    return cmd;
}