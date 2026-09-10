#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include "server.h"
#include "parser.h"
#include "hashtable.h"
#include "persistence.h"
#include <netinet/tcp.h>

#define MAX_CLIENTS 1024
#define READ_CHUNK 1024

typedef struct {
    char buf[PARSER_MAX_LINE];
    size_t len;
    int in_use;
} ClientBuffer;

static ClientBuffer clients[MAX_CLIENTS];
static void client_reset(int fd) {
    if (fd >=0 && fd < MAX_CLIENTS) {
        clients[fd].len = 0;
        clients[fd].in_use = 0;
    }
}
static void handle_command(int fd, HashTable *ht, char *line) {
    Command cmd = parse_command(line);
    switch (cmd.type) {
        case CMD_SET: {
            if (!ht_set(ht, cmd.key, cmd.value)) {
                const char *msg = "ERROR out of memory\n";
                write(fd, msg, strlen(msg));
                break;
            }
            aof_append(line);
            write(fd, "OK\n", 3);
            break;
        }
        case CMD_GET: {
            const char *value = ht_get(ht, cmd.key);
            char response[PARSER_MAX_TOKEN + 2];
            if (value) {
                snprintf(response, sizeof(response), "%s\n", value);
            }
            else {
                snprintf(response, sizeof(response), "(nil)\n");
            }
            write(fd, response, strlen(response));
            break;
        }
        case CMD_DEL: {
            int deleted = ht_delete(ht, cmd.key);
            if (deleted) {
                aof_append(line);
                write(fd, "OK\n", 3);
            }
            else
                write(fd, "(nil)\n", 6);
            break;
        }
        case CMD_TOO_LONG: {
            const char *msg = "ERROR key or value too long\n";
            write(fd, msg, strlen(msg));
            break;
        }
        case CMD_UNKNOWN:
        default: {
            const char *msg = "ERROR unknown command\n";
            write(fd, msg, strlen(msg));
            break;
        }
    }
}

static void feed_client(int fd, HashTable *ht, const char *data, size_t data_len) {
    if (fd < 0 || fd >= MAX_CLIENTS)
        return;
    ClientBuffer *cb = &clients[fd];
    cb->in_use = 1;
 
    size_t offset = 0;
    while (offset < data_len) {
        size_t space = sizeof(cb->buf) - cb->len;
        if (space == 0) {
            const char *msg = "ERROR line too long\n";
            write(fd, msg, strlen(msg));
            cb->len = 0;
            return;
        }
        size_t to_copy = data_len - offset;
        if (to_copy > space)
            to_copy = space;
        memcpy(cb->buf + cb->len, data + offset, to_copy);
        cb->len += to_copy;
        offset += to_copy;

        while (1) {
            char *newline = memchr(cb->buf, '\n', cb->len);
            if (!newline)
                break;
            size_t line_len = (size_t)(newline - cb->buf);
            char line[PARSER_MAX_LINE];
            memcpy(line, cb->buf, line_len);
            line[line_len] = '\0';
            if (line_len >0 && line[line_len-1] == '\r')
                line[line_len - 1] = '\0';
            handle_command(fd, ht, line);

            size_t consumed = line_len + 1;
            size_t remaining = cb->len - consumed;
            memmove(cb->buf, cb->buf + consumed, remaining);
            cb->len= remaining;
        }
    }
    
}

int main(void) {
    HashTable *ht = ht_create(16);
    aof_load(ht);
    aof_open();
    int server_fd = start_server(6379);
    printf("Server listening on port 6379...\n");

   int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        perror("epoll_create1 failed");
        exit(EXIT_FAILURE);
    }
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) < 0) {
        perror("epoll_ctl failed");
        exit(EXIT_FAILURE);
    }

    struct epoll_event events[64];
    char buffer[READ_CHUNK];
    while (1) {
        int num_events = epoll_wait(epoll_fd, events, 64, -1);
        if (num_events < 0) {
            perror("epoll_wait failed");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < num_events; i++) {
            int current_fd = events[i].data.fd;

            if (current_fd == server_fd) {

                int client_fd = accept(server_fd, NULL, NULL);
                if (client_fd < 0) {
                    perror("accept failed");
                    continue;
                }
                if (client_fd >= MAX_CLIENTS) {
                    close(client_fd);
                    continue;
                }

                int flag = 1;
                setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
                printf("Client connected: fd=%d\n", client_fd);

                client_reset(client_fd);
                clients[client_fd].in_use = 1;

                struct epoll_event client_event;
                client_event.events = EPOLLIN;
                client_event.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event);
            }
            else {
                ssize_t bytes_read = read(current_fd, buffer, sizeof(buffer) - 1);
                if (bytes_read <= 0) {
                    printf("Client disconnected: fd=%d\n", current_fd);
                    close(current_fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL);
                    continue;
                }
                feed_client(current_fd, ht, buffer, (size_t)bytes_read);
                
            }
        }
       
    }
   
    aof_close();
    ht_destroy(ht);
    return 0;
}