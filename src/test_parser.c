#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "parser.h"

static int failures = 0;

#define CHECK(cond, description) do { \
    if (!(cond)) { \
        printf("FAIL: %s\n", description); \
        failures++; \
    } else { \
        printf("PASS: %s\n", description); \
    } \
} while (0)

static void test_set_command(void) {
    Command c = parse_command("SET name Anne");
    CHECK(c.type == CMD_SET, "SET is parsed as CMD_SET");
    CHECK(strcmp(c.key, "name") == 0, "SET key is parsed correctly");
    CHECK(strcmp(c.value, "Anne") == 0, "SET value is parsed correctly");
}

static void test_get_command(void) {
    Command c = parse_command("GET name");
    CHECK(c.type == CMD_GET, "GET is parsed as CMD_GET");
    CHECK(strcmp(c.key, "name") == 0, "GET key is parsed correctly");
}

static void test_del_command(void) {
    Command c = parse_command("DEL name");
    CHECK(c.type == CMD_DEL, "DEL is parsed as CMD_DEL");
    CHECK(strcmp(c.key, "name")== 0, "DEL key is parsed correctly");
}

static void test_unknown_command(void) {
    Command c = parse_command("FOO bar");
    CHECK(c.type == CMD_UNKNOWN, "unrecognized verb is CMD_UNKNOWN");
}

static void test_empty_input(void) {
    Command c = parse_command("");
    CHECK(c.type == CMD_UNKNOWN, "empty input is CMD_UNKNOWN");
}

static void test_get_with_no_key(void) {
    Command c = parse_command("GET");
    CHECK(c.type == CMD_GET, "GET with no key still parses as CMD_GET");
    CHECK(c.key[0] == '\0', "GET with no key leaves key empty");
}

static void test_set_with_no_value(void) {
    Command c = parse_command("SET onlykey");
    CHECK(c.type == CMD_SET, "SET with no value still parses as CMD_SET");
    CHECK(strcmp(c.key, "onlykey") == 0, "SET with no value still captures the key");
    CHECK(c.value[0] == '\0', "SET with no value leaves value empty");
}

void print_commad(const char *label, Command cmd) {
    printf("%s -> type=%d, key='%s', value='%s'\n", label, cmd.type, cmd.key, cmd.value);

}

static void test_oversized_key_is_rejected(void) {
    char input[600];
    memset(input, 'x', sizeof(input) - 1);
    input[sizeof(input)-1] = '\0';
    memcpy(input, "GET ", 4);
    Command c = parse_command(input);
    CHECK(c.type == CMD_TOO_LONG, "oversized key is rejected as CMD_TOO_LONG, not overflowed");
}

static void test_oversized_line_is_rejected(void) {
    char input[PARSER_MAX_LINE+10];
    memset(input, 'a', sizeof(input) - 1);
    input[sizeof(input)-1] = '\0';

    Command c = parse_command(input);
    CHECK(c.type == CMD_TOO_LONG, "line at/over PARSER_MAX_LINE is rejected as CMD_TOO_LONG");
}

static void test_extra_whitespace_tokens(void) {
    Command c = parse_command("SET a b extra ignored");
    CHECK(c.type == CMD_SET, "extra trailing tokens don't break parsing");
    CHECK(strcmp(c.key, "a") == 0, "key is still parsed correctly with trailing tokens");
    CHECK(strcmp(c.value, "b") == 0, "value takes only the third token, ignoring the rest");
}

int main(void) {
    test_set_command();
    test_get_command();
    test_del_command();
    test_unknown_command();
    test_empty_input();
    test_get_with_no_key();
    test_set_with_no_value();
    test_oversized_key_is_rejected();
    test_oversized_line_is_rejected();
    test_extra_whitespace_tokens();
    printf("\n%d failures(s)\n", failures);
    return failures == 0 ? 0 : 1;
}