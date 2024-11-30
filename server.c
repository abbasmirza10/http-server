#include "http.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUFFER_SIZE 4096



// Generate HTTP response based on requested path
char *generate_response(const char *path) {
    char buffer[BUFFER_SIZE];
    char *response = NULL;
    int file_fd = open(path, O_RDONLY);
    if (file_fd < 0) {
        response = strdup("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
    } else {
        const char *content_type = "text/plain"; // Default content type
        const char *file_extension = strrchr(path, '.');
        if (file_extension != NULL) {
            if (strcmp(file_extension, ".html") == 0) {
                content_type = "text/html";
            } else if (strcmp(file_extension, ".png") == 0) {
                content_type = "image/png";
            }
        }
        struct stat st;
        fstat(file_fd, &st);
        off_t file_size = st.st_size;
        snprintf(buffer, BUFFER_SIZE, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n", content_type, file_size);
        response = strdup(buffer);
        if (response == NULL) {
            perror("malloc");
            close(file_fd);
            return NULL;
        }
        ssize_t bytes_read;
        while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
            strncat(response, buffer, bytes_read);
        }
        close(file_fd);
    }
    return response;
}

// Send response to client
void send_response(int sockfd, const char *response) {
    if (response == NULL) {
        return;
    }
    ssize_t response_len = strlen(response);
    ssize_t total_sent = 0;
    ssize_t sent;
    while (total_sent < response_len) {
        sent = write(sockfd, response + total_sent, response_len - total_sent);
        if (sent < 0) {
            perror("write");
            break;
        }
        total_sent += sent;
    }
}

// Close connection and clean up
void close_connection(int sockfd, HTTPRequest *req) {
    close(sockfd);
    if (req != NULL) {
        httprequest_destroy(req);
    }
}


void *client_thread(void *vptr) {
    int sockfd = *((int *)vptr);
    free(vptr);

    char buffer[BUFFER_SIZE];
    ssize_t len = read(sockfd, buffer, sizeof(buffer) - 1);
    if (len < 0) {
        perror("read");
        close_connection(sockfd, NULL);
        return NULL;
    }
    buffer[len] = '\0';

    HTTPRequest req;
    memset(&req, 0x00, sizeof(HTTPRequest));
    ssize_t header_len = httprequest_parse_headers(&req, buffer, len);
    if (header_len < 0) {
        printf("Failed to parse headers\n");
        close_connection(sockfd, &req);
        return NULL;
    }

    const char *action = httprequest_get_action(&req);
    const char *path = httprequest_get_path(&req);
    const char *version = httprequest_get_header(&req, "Host");
    printf("Received request: %s %s %s\n", action, path, version);

    char *response = generate_response(path);
    send_response(sockfd, response);
    free(response);

    close_connection(sockfd, &req);
    return NULL;
}


int main(int argc, char *argv[]) {
  //Completed function - do not modify
  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    return 1;
  }
  int port = atoi(argv[1]);
  printf("Binding to port %d. Visit http://localhost:%d/ to interact with your server!\n", port, port);

  // socket:
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // bind:
  struct sockaddr_in server_addr, client_address;
  memset(&server_addr, 0x00, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);  
  bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr));

  // listen:
  listen(sockfd, 10);

  // accept:
  socklen_t client_addr_len;
  while (1) {
    int *fd = malloc(sizeof(int));
    client_addr_len = sizeof(struct sockaddr_in);
    *fd = accept(sockfd, (struct sockaddr *)&client_address, &client_addr_len);
    printf("Client connected (fd=%d)\n", *fd);

    pthread_t tid;
    pthread_create(&tid, NULL, client_thread, fd);
    pthread_detach(tid);
  }

  return 0;
}