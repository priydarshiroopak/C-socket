#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_HEADER_LENGTH 4096

struct Request {
  char host[MAX_HEADER_LENGTH];
  char connection[MAX_HEADER_LENGTH];
  char date[MAX_HEADER_LENGTH];
  char accept[MAX_HEADER_LENGTH];
  char accept_language[MAX_HEADER_LENGTH];
  char if_modified_since[MAX_HEADER_LENGTH];
  char content_language[MAX_HEADER_LENGTH];
  int content_length;
  char content_type[MAX_HEADER_LENGTH];
};

struct Response {
  char expires[MAX_HEADER_LENGTH];
  char cache_control[MAX_HEADER_LENGTH];
  char content_language[MAX_HEADER_LENGTH];
  int content_length;
  char content_type[MAX_HEADER_LENGTH];
  char last_modified[MAX_HEADER_LENGTH];
};

void parse_request(char* request, struct Request* req) {
  char* line = strtok(request, "\r\n");
  while (line != NULL) {
    if (strncasecmp(line, "Host:", 5) == 0) {
      strncpy(req->host, line + 6, MAX_HEADER_LENGTH - 1);
    } else if (strncasecmp(line, "Connection:", 11) == 0) {
      strncpy(req->connection, line + 11, MAX_HEADER_LENGTH - 1);
    } else if (strncasecmp(line, "Date:", 5) == 0) {
      strncpy(req->date, line + 5, MAX_HEADER_LENGTH - 1);
    } else if (strncasecmp(line, "Accept:", 7) == 0) {
      strncpy(req->accept, line + 7, MAX_HEADER_LENGTH - 1);
    } else if (strncasecmp(line, "Accept-Language:", 16) == 0) {
      strncpy(req->accept_language, line + 16, MAX_HEADER_LENGTH - 1);
    } else if (strncasecmp(line, "If-Modified-Since:", 18) == 0) {
      strncpy(req->if_modified_since, line + 18, MAX_HEADER_LENGTH - 1);
    } else if (strncasecmp(line, "Content-Language:", 17) == 0) {
      strncpy(req->content_language, line + 17, MAX_HEADER_LENGTH - 1);
    } else if (strncasecmp(line, "Content-Length:", 15) == 0) {
      sscanf(line + 15, "%d", &req->content_length);
    } else if (strncasecmp(line, "Content-Type:", 13) == 0) {
      strncpy(req->content_type, line + 13, MAX_HEADER_LENGTH - 1);
    }
    line = strtok(NULL, "\r\n");
  }
}

int main() {
    char request[] = "GET / HTTP/1.1\r\nHost: www.google.com\r\nConnection: keep-alive\r\nDate: Mon, 23 May 2005 22:38:34 GMT\r\nAccept: text/html, image/gif, image/jpeg, *; q=.2, */*; q=.2\r\nAccept-Language: en-us\r\nIf-Modified-Since: Sat, 29 Oct 1994 19:43:31 GMT\r\nContent-Language: en\r\nContent-Length: 9\r\nContent-Type: text/plain\r\n\r\nHi there!";
    struct Request req;
    parse_request(request, &req);
    printf("Host: %s", req.host);
    printf("Connection: %s", req.connection);
}