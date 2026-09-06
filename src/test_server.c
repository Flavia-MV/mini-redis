#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "server.h"
#include <unistd.h>

int main(void) {
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
    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

    if (bytes_read < 0)  {
        perror("read failed");
        exit(EXIT_FAILURE);
    }

    if (bytes_read == 0) {
        printf("Client disconnected before sending data.\n");
        close(client_fd);
        return 0;
    }

    buffer[bytes_read] = '\0';
    printf("Received: %s\n", buffer);
    write(client_fd, "OK\n", 3);

    close(client_fd);
    close(server_fd);
    return 0;
}