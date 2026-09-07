#include <stdio.h>
#include "parser.h"

void print_commad(const char *label, Command cmd) {
    printf("%s -> type=%d, key='%s', value='%s'\n", label, cmd.type, cmd.key, cmd.value);

}

int main(void) {
    Command c1 = parse_command("SET nume Flavia");
    print_commad("SET nume Flavia", c1);
    Command c2 = parse_command("GET nume");
    print_commad("GET nume", c2);
    Command c3 = parse_command("DEL nume");
    print_commad("DEL nume", c3);
    Command c4 = parse_command("TEST");
    print_commad("TEST", c4);
    Command c5 = parse_command("GET");
    print_commad("GET (no key)", c5);
    return 0;
}