#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "server.h"
#include "parser.h"
#include "hashtable.h"

int main(void) {
    HashTable *ht = ht_create(16);
    int server_fd = start_server(6379);
    printf("Server listening on port 6379...\n");

    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);
    int client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_len);
    if (client_fd < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    printf("Client connected\n");

    char buffer[1024];
    while (1) {
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) {
           printf("Client disconnected\n");
           break;
        }
        buffer[bytes_read] = '\0';
        buffer[strcspn(buffer, "\r\n")] = '\0';

        Command cmd = parse_command(buffer);
        switch (cmd.type)
        {
        case CMD_SET: {
            ht_set(ht, cmd.key, cmd.value);
            write(client_fd, "OK\n", 3);
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
            write(client_fd, response, strlen(response));
            break;
        }
        case CMD_DEL: {
            int deleted = ht_delete(ht, cmd.key);
            if (deleted) 
                write(client_fd, "OK\n", 3);
            else
                write(client_fd, "(nil)\n", 6);
            break;
        }
        case CMD_UNKNOWN: {
            const char *msg = "ERROR unknown command\n";
            write(client_fd, msg, strlen(msg));
            break;
        }
        default:
            break;
        }
    }
   


    close(client_fd);
    close(server_fd);
    ht_destroy(ht);
    return 0;
}