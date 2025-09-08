#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define SERVER_IP "192.168.1.100"  // Change to server's IP

int main(int argc, char const* argv[])
{
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    char* hello = "Hello from client";
    char buffer[1024] = { 0 };

    // Create socket
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 address from text to binary
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address / Address not supported\n");
        return -1;
    }

    // Connect to server
    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr,
                          sizeof(serv_addr))) < 0) {
        printf("\nConnection Failed\n");
        return -1;
    }

    // Send message
    send(client_fd, hello, strlen(hello), 0);
    printf("Hello message sent\n");

    // Receive server reply
    valread = read(client_fd, buffer, 1024 - 1);
    buffer[valread] = '\0';
    printf("Server: %s\n", buffer);

    close(client_fd);
    return 0;
}
