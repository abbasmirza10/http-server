#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#include "http.h"

#define BUFFER_SIZE 4096



char* parse_request_line(HTTPRequest *req, char *line_start) {
  char *line_end = strstr(line_start, "\r\n");
  if (line_end == NULL) {
    return NULL;
  }
  *line_end = '\0'; 
  char *action_end = strchr(line_start, ' ');
  if (action_end == NULL) {
    return NULL;
  }
  *action_end = '\0';
  req->action = strdup(line_start);
  char *path_end = strchr(action_end + 1, ' ');
  if (path_end == NULL) {
    return NULL;
  }
  *path_end = '\0';
  req->path = strdup(action_end + 1);
  req->version = strdup(path_end + 1);
  *line_end = '\r'; 
  return line_end + 2;
}

HTTPHeader* create_header(char *line_start) {
  char *colon_pos = strchr(line_start, ':');
  if (colon_pos == NULL) {
    return NULL;
  }
  *colon_pos = '\0';  
  char *value_start = colon_pos + 1;
  while (*value_start == ' ') {
    value_start++;
  }
  HTTPHeader *header = malloc(sizeof(HTTPHeader));
  header->key = strdup(line_start);
  header->value = strdup(value_start);
  header->next = NULL;
  return header;
}

char* parse_header_line(HTTPHeader **current_header, char *line_start) {
  char *line_end = strstr(line_start, "\r\n");
  if (line_end == NULL) {
      return NULL;
  }
  *line_end = '\0';  
  *current_header = create_header(line_start);
  if (*current_header == NULL) {
      return NULL;
  }
  *line_end = '\r';  
  return line_end + 2;
}

ssize_t httprequest_parse_headers(HTTPRequest *req, char *buffer, ssize_t buffer_len) {
    char *line_start = buffer;
    line_start = parse_request_line(req, line_start);
    if (line_start == NULL) {
        return -1;
    }
    req->headers = NULL;
    HTTPHeader **current_header = &(req->headers);
    while ((line_start = parse_header_line(current_header, line_start)) != NULL) {
        current_header = &((*current_header)->next);
    }
    return line_start - buffer;
}


ssize_t read_headers(int sockfd, char *buffer, ssize_t *total_received) {
    ssize_t received;
    while ((*total_received) < BUFFER_SIZE - 1) {
        received = read(sockfd, buffer + *total_received, BUFFER_SIZE - *total_received - 1);
        if (received < 0) {
            return -1;
        }
        *total_received += received;
        buffer[*total_received] = '\0';
        if (strstr(buffer, "\r\n\r\n") != NULL) {
            break;
        }
    }
    return *total_received;
}

char *allocate_and_copy_headers(char *buffer, ssize_t total_received) {
    char *request_str = (char *)malloc(total_received + 1);
    if (request_str == NULL) {
        return NULL;
    }
    strncpy(request_str, buffer, total_received);
    request_str[total_received] = '\0';
    return request_str;
}

ssize_t read_message_body(int sockfd, HTTPRequest *req, const char *content_length_str) {
    int content_length = atoi(content_length_str);
    if (content_length <= 0) { // Check for invalid content-length
        return 0; // No payload to read
    }
    char *body_buffer = (char *)malloc(content_length + 1);
    if (body_buffer == NULL) {
        return -1;
    }
    ssize_t body_received = 0;
    ssize_t received;
    while (body_received < content_length) {
        received = read(sockfd, body_buffer + body_received, content_length - body_received);
        if (received <= 0) {
            free(body_buffer);
            return -1;
        }
        body_received += received;
    }
    body_buffer[content_length] = '\0';
    req->payload = body_buffer;
    return 0;
}

ssize_t httprequest_read(HTTPRequest *req, int sockfd) {
    if (req == NULL || sockfd < 0) {
        return -1;
    }
    char buffer[BUFFER_SIZE];
    ssize_t total_received = 0;
    ssize_t received = read_headers(sockfd, buffer, &total_received);
    if (received < 0) {
        return -1;
    }
    char *request_str = allocate_and_copy_headers(buffer, total_received);
    if (request_str == NULL) {
        return -1;
    }
    httprequest_parse_headers(req, request_str, total_received);
    const char *content_length_str = httprequest_get_header(req, "Content-Length");
    if (content_length_str != NULL) {
        if (read_message_body(sockfd, req, content_length_str) < 0) {
            free(request_str);
            return -1;
        }
    } else {
        req->payload = NULL; 
    }
    free(request_str);
    return total_received;
}


const char *httprequest_get_action(HTTPRequest *req) {
  if (req == NULL) {
    return NULL;
  }
  return req->action;
}

const char *httprequest_get_header(HTTPRequest *req, const char *key) {
  if (req == NULL) {
    return NULL;
  }
  for (HTTPHeader *header = req->headers; header != NULL; header = header->next) {
    if (strcmp(header->key, key) == 0) {
      return header->value;
    }
  }
  return NULL;
}

const char *httprequest_get_path(HTTPRequest *req) {
  if (req == NULL) {
    return NULL;
  }
  return req->path;
}

void httprequest_destroy(HTTPRequest *req) {
  if (req == NULL) {
    return;
  }
  for (HTTPHeader *header = req->headers; header != NULL; ) {
    HTTPHeader *next_header = header->next;
    free(header->key);
    free(header->value);
    free(header);
    header = next_header;
  }
}

