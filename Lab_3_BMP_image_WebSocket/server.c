#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> 
#define PORT 8080 
#define SA struct sockaddr 

void send_bmp(int connfd, const char *filename) 
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("Failed to open BMP file");
        // Send file size = 0 to indicate failure
        long filesize = 0;
        write(connfd, &filesize, sizeof(filesize));
        return;
    }

    // Find file size
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Send filesize first
    if (write(connfd, &filesize, sizeof(filesize)) != sizeof(filesize)) {
        perror("Failed to send file size");
        fclose(fp);
        return;
    }

    // Send file data in chunks
    char buffer[4096];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        if (write(connfd, buffer, bytes_read) != bytes_read) {
            perror("Failed to send file data");
            fclose(fp);
            return;
        }
    }

    printf("BMP file '%s' sent successfully.\n", filename);
    fclose(fp);
}

int main() 
{ 
    int sockfd, connfd, len; 
    struct sockaddr_in servaddr, cli; 

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
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 

    // Bind socket
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        perror("Socket bind failed");
        exit(EXIT_FAILURE);
    } 
    printf("Socket successfully binded..\n"); 

    // Listen
    if ((listen(sockfd, 5)) != 0) { 
        perror("Listen failed");
        exit(EXIT_FAILURE);
    } 
    printf("Server listening..\n"); 

    len = sizeof(cli); 

    // Accept client connection
    connfd = accept(sockfd, (SA*)&cli, &len); 
    if (connfd < 0) { 
        perror("Server accept failed");
        exit(EXIT_FAILURE);
    } 
    printf("Server accepted the client...\n"); 

    // First receive filename length (int)
    int filename_len;
    ssize_t n = read(connfd, &filename_len, sizeof(filename_len));
    if (n != sizeof(filename_len) || filename_len <= 0 || filename_len > 256) {
        printf("Invalid filename length received.\n");
        close(connfd);
        close(sockfd);
        return 1;
    }

    // Receive filename string
    char filename[257] = {0};
    n = read(connfd, filename, filename_len);
    if (n != filename_len) {
        printf("Failed to receive full filename.\n");
        close(connfd);
        close(sockfd);
        return 1;
    }
    filename[filename_len] = '\0'; // Null terminate

    printf("Client requested file: %s\n", filename);

    // Send the requested BMP file
    send_bmp(connfd, filename);

    close(connfd);
    close(sockfd);

    return 0;
}
