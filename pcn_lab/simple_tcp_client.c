#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int client_fd;
    struct sockaddr_in server_addr;
    char *msg = "Hello Server";

    client_fd = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080); 


    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
// inet_pton() converts an IP address from text format to binary network format.

    connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
// connect() is used by a client socket to establish a connection with a server.
    write(client_fd, msg, strlen(msg));
// sends the message stored in msg through the connected TCP socket.
    close(client_fd);

    return 0;
}