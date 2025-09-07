#include <arpa/inet.h>   // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>     // bzero()
#include <sys/socket.h>
#include <unistd.h>      // read(), write(), close()

#define PORT 8080
#define SA struct sockaddr

int main()
{
    int sockfd;
    struct sockaddr_in servaddr;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket successfully created..\n");

    bzero(&servaddr, sizeof(servaddr));

    // Assign IP and PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    // Connect to server
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        perror("Connection with the server failed");
        exit(EXIT_FAILURE);
    }
    printf("Connected to the server..\n");

    // Get filename from user
    char filename[256];
    printf("Enter BMP filename to request: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        fprintf(stderr, "Failed to read filename.\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Remove newline if present
    size_t len = strlen(filename);
    if (filename[len-1] == '\n') filename[len-1] = '\0';
    len = strlen(filename); // update length after trim

    if (len == 0) {
        fprintf(stderr, "Filename cannot be empty.\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Send filename length first (int)
    int filename_len = (int)len;
    if (write(sockfd, &filename_len, sizeof(filename_len)) != sizeof(filename_len)) {
        perror("Failed to send filename length");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Send filename string
    if (write(sockfd, filename, filename_len) != filename_len) {
        perror("Failed to send filename");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Receive file size (long)
    long filesize = 0;
    ssize_t n = read(sockfd, &filesize, sizeof(filesize));
    if (n != sizeof(filesize)) {
        perror("Failed to read file size");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (filesize == 0) {
        printf("Server: File not found or cannot be opened.\n");
        close(sockfd);
        exit(EXIT_SUCCESS);
    }

    printf("Receiving file '%s' of size %ld bytes\n", filename, filesize);

    // Open file to write received data
    FILE *fp = fopen("received.bmp", "wb");
    if (!fp) {
        perror("Failed to open file for writing");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Receive file data
    char buffer[4096];
    long total_received = 0;
    while (total_received < filesize) {
        ssize_t bytes_to_read = sizeof(buffer);
        if (filesize - total_received < bytes_to_read) {
            bytes_to_read = filesize - total_received;
        }
        n = read(sockfd, buffer, bytes_to_read);
        if (n <= 0) {
            perror("Failed to read file data");
            break;
        }
        fwrite(buffer, 1, n, fp);
        total_received += n;
    }

    if (total_received == filesize) {
        printf("File received successfully and saved as 'received.bmp'.\n");
    } else {
        printf("File transfer incomplete. Received %ld bytes.\n", total_received);
    }

    fclose(fp);
    close(sockfd);

    return 0;
}
