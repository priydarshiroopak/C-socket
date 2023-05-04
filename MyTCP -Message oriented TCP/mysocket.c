// # Roopak Priydarshi  - 20CS30042
// # Atishay Jain       - 20CS30008
#include "mysocket.h"
struct table *sender_table, *receiver_table;

// Mutexes
pthread_mutex_t send_mutex, recv_mutex, fd_mutex;

// sockfd accepted from the server
int _sockfd;

// mutex condition variables
pthread_cond_t send_full, recv_not_empty, send_empty;

void init_table(struct table *t)
{
    memset(t->size, 0, sizeof(t->size));
    memset(t->data, 0, sizeof(t->data));
    t->first = 0;
    t->count = 0;
}

int my_socket(int domain, int type, int protocol)
{
    int fd = socket(domain, type, protocol);
    if (fd < 0)
        return fd;

    // initiate table
    sender_table = (struct table *)malloc(sizeof(struct table));
    receiver_table = (struct table *)malloc(sizeof(struct table));

    init_table(sender_table);
    init_table(receiver_table);

    // initiate mutex
    pthread_mutex_init(&send_mutex, NULL);
    pthread_mutex_init(&recv_mutex, NULL);
    pthread_mutex_init(&fd_mutex, NULL);

    // init condition variable
    pthread_cond_init(&send_full, NULL);
    pthread_cond_init(&recv_not_empty, NULL);
    pthread_cond_init(&send_empty, NULL);

    _sockfd = -1;

    // Thread arguments
    pthread_t sender, receiver;

    // initiate thread
    pthread_create(&sender, NULL, send_thread, NULL);
    pthread_create(&receiver, NULL, recv_thread, NULL);

    return fd;
}

int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    return bind(sockfd, addr, addrlen);
}

int my_listen(int sockfd, int backlog)
{
    return listen(sockfd, backlog);
}

int my_accept(int sockfd, struct sockaddr *restrict addr, socklen_t *restrict addrlen)
{
    int fd = accept(sockfd, addr, addrlen);
    if (fd < 0)
        return fd;
    pthread_mutex_lock(&fd_mutex);
    _sockfd = fd;
    pthread_mutex_unlock(&fd_mutex);
    return fd;
}

int my_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int ret = connect(sockfd, addr, addrlen);
    if (ret < 0)
        return ret;
    pthread_mutex_lock(&fd_mutex);
    _sockfd = sockfd;
    pthread_mutex_unlock(&fd_mutex);
    return ret;
}

ssize_t my_send(int sockfd, const void *buf, size_t len, int flags)
{
    pthread_mutex_lock(&send_mutex);
    if (sender_table->count == MAX_SIZE)
    {
        pthread_cond_wait(&send_full, &send_mutex);
    }
    len = len > 5000 ? 5000 : len;
    int ind = (sender_table->first + sender_table->count) % MAX_SIZE;
    strncpy(sender_table->data[ind], buf, len);
    sender_table->size[ind] = len;
    sender_table->count++;
    pthread_mutex_unlock(&send_mutex);
    return len;
}

ssize_t my_recv(int sockfd, void *buf, size_t len, int flags)
{
    pthread_mutex_lock(&recv_mutex);
    if (receiver_table->count == 0)
    {
        pthread_cond_wait(&recv_not_empty, &recv_mutex);
    }
    int ret = receiver_table->size[receiver_table->first];
    ret = ret < len ? ret : len;
    strncpy(buf, receiver_table->data[receiver_table->first], ret);
    receiver_table->count--;
    receiver_table->first = (receiver_table->first + 1) % MAX_SIZE;
    pthread_mutex_unlock(&recv_mutex);
    return ret;
}

int my_close(int fd)
{
    // send all the messsages that are in the sender table
    pthread_mutex_lock(&send_mutex);
    pthread_cond_wait(&send_empty, &send_mutex);
    pthread_mutex_unlock(&send_mutex);

    // close the sockfd
    pthread_mutex_lock(&fd_mutex);
    _sockfd = -1;
    pthread_mutex_unlock(&fd_mutex);

    // clear table for receiver required for server for next connection
    pthread_mutex_lock(&recv_mutex);
    memset(receiver_table->size, 0, sizeof(receiver_table->size));
    memset(receiver_table->data, 0, sizeof(receiver_table->data));
    receiver_table->first = 0;
    receiver_table->count = 0;
    pthread_mutex_unlock(&recv_mutex);

    // no need to do for the sender table considering servers' side
    return close(fd);
}

void *send_thread(void *arg)
{
    char buf[MAX_MSG_SIZE + 2];
    int sent, sending, ret, len;
    while (1)
    {
        pthread_mutex_lock(&send_mutex);
        if (sender_table->count > 0)
        {
            len = sender_table->size[sender_table->first];
            strncpy(buf + 2, sender_table->data[sender_table->first], len);
            sender_table->count--;
            sender_table->first = (sender_table->first + 1) % MAX_SIZE;
            pthread_cond_signal(&send_full);
            pthread_mutex_unlock(&send_mutex);
            buf[0] = len >> 8;
            buf[1] = len & ((1 << 8) - 1);
            len += 2;
            sent = 0;
            while (len > 0)
            {
                sending = len > 1000 ? 1000 : len;
                ret = 0;
                // no need to use mutex lock as _sockfd will become -1 only after send table is empty
                ret = send(_sockfd, buf + sent, sending, 0);
                if (ret < 0)
                {
                    // error
                    break;
                }
                else
                {
                    sent += ret;
                    len -= ret;
                }
            }
        }
        else
        {
            pthread_cond_signal(&send_empty);
            pthread_mutex_unlock(&send_mutex);
        }
    }
}

void *recv_thread(void *arg)
{
    char buf[MAX_MSG_SIZE];
    char len_buf[2];
    int received, ret, len, sockfd;
    int flag = 0;
    while (1)
    {
        pthread_mutex_lock(&recv_mutex);
        if (receiver_table->count < MAX_SIZE)
        {
            pthread_mutex_unlock(&recv_mutex);
            received = 0;
            while (received < 2)
            {
                ret = 0;
                pthread_mutex_lock(&fd_mutex);
                if (_sockfd != -1)
                {
                    ret = recv(_sockfd, len_buf, 2 - received, MSG_DONTWAIT);
                }
                pthread_mutex_unlock(&fd_mutex);
                if (ret == -1) // when there is no message
                    ret = 0;
                received += ret;
            }
            if (flag == 1)
            {
                flag = 0;
                continue;
            }
            if (received == 2)
            {
                len = ((int)len_buf[0] << 8) + len_buf[1];
                received = 0;
                while (len > 0)
                {
                    pthread_mutex_lock(&fd_mutex);
                    if (_sockfd != -1)
                    {
                        ret = recv(_sockfd, buf + received, len, MSG_DONTWAIT);
                    }
                    pthread_mutex_unlock(&fd_mutex);
                    if (ret == -1)
                        ret = 0;
                    received += ret;
                    len -= ret;
                }
            }
            else
            {
                received = 0;
            }

            // even if there is an error, we still add the data to the table, so that it will not receive the data of the next my_send call
            pthread_mutex_lock(&recv_mutex);
            strncpy(receiver_table->data[(receiver_table->first + receiver_table->count) % MAX_SIZE], buf, received);
            receiver_table->size[(receiver_table->first + receiver_table->count) % MAX_SIZE] = received;
            receiver_table->count++;
            pthread_cond_signal(&recv_not_empty);
            pthread_mutex_unlock(&recv_mutex);
        }
        else
        {
            pthread_mutex_unlock(&recv_mutex);
        }
    }
}