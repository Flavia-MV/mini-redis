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

int main(void) {
    HashTable *ht = ht_create(16);
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
    char buffer[1024];
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
                printf("Client connected: fd=%d\n", client_fd);

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
                buffer[bytes_read] = '\0';
                buffer[strcspn(buffer, "\r\n")] = '\0';

                Command cmd = parse_command(buffer);
                switch (cmd.type)
                {
                case CMD_SET: {
                    ht_set(ht, cmd.key, cmd.value);
                    write(current_fd, "OK\n", 3);
                    break;
                }
                case CMD_GET: {
                    const char *value = ht_get(ht, cmd.key);
                    char response[512];
                    if (value) {
                        snprintf(response, sizeof(response), "%s\n", value);
                    }
                    else {
                        snprintf(response, sizeof(response), "(nil)\n");
                    }
                    write(current_fd, response, strlen(response));
                    break;
                }
                case CMD_DEL: {
                    int deleted = ht_delete(ht, cmd.key);
                    if (deleted) 
                        write(current_fd, "OK\n", 3);
                    else
                        write(current_fd, "(nil)\n", 6);
                    break;
                }
                case CMD_UNKNOWN: {
                    const char *msg = "ERROR unknown command\n";
                    write(current_fd, msg, strlen(msg));
                    break;
                }
                default:
                    break;
                }
            }
        }
       
    }
   
    ht_destroy(ht);
    return 0;
}