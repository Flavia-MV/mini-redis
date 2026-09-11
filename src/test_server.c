#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "server.h"
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

static int failures = 0;

#define CHECK(cond, description) do { \
    if (!(cond)) { \
        printf("FAIL: %s\n", description); \
        failures++; \
    } else { \
        printf("PASS: %s\n", description); \
    } \
} while (0)

static void run_client(int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int connected = 0;
    for (int attempt = 0; attempt < 50; attempt++) {
        if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
            connected = 1;
            break;
        }
        usleep(10000);
    }
    if (!connected) {
        _exit(1);
    }

    const char *msg = "hello\n";
    write(fd, msg, strlen(msg));

    char response[64];
    ssize_t n = read(fd, response, sizeof(response) - 1);
    if (n <= 0)
        _exit(1);
    response[n] = '\0';
    close(fd);
    _exit(strcmp(response, "OK\n") == 0 ? 0 : 1);
}

static void test_accept_and_echo(void) {
    int server_fd = start_server(6390);

    pid_t pid = fork();
    if (pid == 0)
        run_client(6390);
    int client_fd = accept(server_fd, NULL, NULL);
    CHECK(client_fd >= 0, "server accepts an incoming connection");

    char buffer[64];
    ssize_t n = read(client_fd, buffer, sizeof(buffer)-1);
    CHECK(n>0, "server reads data sent by the client");
    if (n > 0) {
        buffer[n] = '\0';
        CHECK(strcmp(buffer, "hello\n") == 0, "server receives the exact bytes the client sent");
    }
    write(client_fd, "OK\n", 3);

    close(client_fd);
    close(server_fd);

    int status = 0;
    waitpid(pid, &status, 0);
    CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0, "client received the expected response");
}

static void test_immediate_rebind(void) {
    int fd1 = start_server(6391);
    close(fd1);

    int fd2 = start_server(6391);
    CHECK(fd2 >=0, "port can be rebound immediately after close (SO_REUSEADDR)");
    close(fd2);
}

int main(void) {
    test_accept_and_echo();
    test_immediate_rebind();

    printf("\n%d failures\n", failures);
    return failures == 0 ? 0 : 1;
}