#ifndef PARSER_H
#define PARSER_H

#define PARSER_MAX_LINE 1024
#define PARSER_MAX_TOKEN 256

typedef enum {
    CMD_SET,
    CMD_GET,
    CMD_DEL,
    CMD_UNKNOWN,
    CMD_TOO_LONG
} CommandType;

typedef struct {
    CommandType type;
    char key[PARSER_MAX_TOKEN];
    char value[PARSER_MAX_TOKEN];
} Command;

Command parse_command(const char *input);

#endif // !PARSER_H
