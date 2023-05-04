// # Roopak Priydarshi  - 20CS30042
// # Atishay Jain       - 20CS30008
#ifndef SOCK_HEADER
#define SOCKHEADER
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MAX_SIZE 10
#define MAX_MSG_SIZE 5000
#define SOCK_MyTCP SOCK_STREAM

int my_socket(int domain, int type, int protocol);
int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int my_listen(int sockfd, int backlog);
int my_accept(int sockfd, struct sockaddr *restrict addr, socklen_t *restrict addrlen);
int my_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
ssize_t my_send(int sockfd, const void *buf, size_t len, int flags);
ssize_t my_recv(int sockfd, void *buf, size_t len, int flags);
int my_close(int fd);

// thread function
void *send_thread(void *arg);
void *recv_thread(void *arg);

// defined data structures
// Table of sent and received data implemented as a circular queue
struct table
{
    ssize_t size[MAX_SIZE];
    char data[MAX_SIZE][MAX_MSG_SIZE];
    int first;
    int count;
};

#endif