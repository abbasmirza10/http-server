#pragma once
#include <stdint.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _HTTPHeader {
  char *key;
  char *value;
  struct _HTTPHeader *next;
};
typedef struct _HTTPHeader HTTPHeader;


struct _HTTPRequest {
  const char *action;
  const char *path;
  const char *version;
  const void *payload;
  //char *header; //declared to hold headers
  HTTPHeader *headers;  // Replace char *header with HTTPHeader *headers  

  // You may want to add more to this struct (ex: to hold the headers).
  // ...however, you MUST keep the `action`, `path`, `version`, and `payload` members
  //    so our test cases can inspect the contents of them without making you write
  //    a bunch of getter functions. :)
};
typedef struct _HTTPRequest HTTPRequest;


ssize_t httprequest_read(HTTPRequest *req, int sockfd);
ssize_t httprequest_parse_headers(HTTPRequest *req, char *buffer, ssize_t buffer_len);
const char *httprequest_get_action(HTTPRequest *req);
const char *httprequest_get_header(HTTPRequest *req, const char *key);
const char *httprequest_get_path(HTTPRequest *req);
void httprequest_destroy(HTTPRequest *req);

//HELPERS
char* parse_request_line(HTTPRequest *req, char *line_start);

HTTPHeader* create_header(char *line_start);

char* parse_header_line(HTTPHeader **current_header, char *line_start);

ssize_t read_headers(int sockfd, char *buffer, ssize_t *total_received);

char *allocate_and_copy_headers(char *buffer, ssize_t total_received);

ssize_t read_message_body(int sockfd, HTTPRequest *req, const char *content_length_str);

ssize_t httprequest_read(HTTPRequest *req, int sockfd);


#ifdef __cplusplus
}
#endif