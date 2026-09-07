#ifndef PARSER_H
#define PARSER_H


typedef enum {
    CMD_SET,
    CMD_GET,
    CMD_DEL,
    CMD_UNKNOWN
} CommandType;

typedef struct {
    CommandType type;
    char key[256];
    char value[256];
} Command;

Command parse_command(const char *input);

#endif // !PARSER_H
