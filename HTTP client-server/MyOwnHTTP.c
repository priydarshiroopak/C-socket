#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define BUFFSIZE 50000    // assumed total header size < BUFFSIZE, i.e. < 48 kB
#define MAX_CLIENTS 5
#define MAXLINE 1024
#define SMALLBUFF 256

char leftover[BUFFSIZE];
int leftover_size = 0;
char *h_GET[] = {"Host:", "Date:", "Accept:", "Accept-Language:", "If-Modified-Since:", "Connection:"};
char *h_PUT[] = {"Host:", "Date:", "Content-Type:", "Content-Length:", "Connection:"};
int h_GET_ctr = 6, h_PUT_ctr = 5;
char homedir[SMALLBUFF];

int min(int a, int b) {    // returns minimum of two integers
    return a < b ? a : b;
}

int max(int a, int b) {    // returns maximum of two integers
    return a > b ? a : b;
}

int strcmpi(char *a, char *b) {    // case insensitive string comparison
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (tolower(a[i]) != tolower(b[i]))
            return 1;
        i++;
    }
    if (a[i] != '\0' || b[i] != '\0')
        return 1;
    return 0;
}

void recvHTTP(int sockfd, char *buffer, int size) {
    buffer[0] = '\0';
    int curr_len = 0, linebreaks = 0;
    do {
        int len = recv(sockfd, buffer + curr_len, min(size - curr_len, MAXLINE), 0);
        if (len < 0) {
            perror("Error reading from socket\n");
            exit(1);
        } else if (!len) {
            break;
        }

        for (int i = 0; i < len; i++) {
            char ch = buffer[curr_len + i];
            if (ch == '\n') {
                linebreaks++;
                if (linebreaks == 2) {
                    buffer[curr_len + i - 1] = '\0';
                    strncpy(leftover, buffer + curr_len + i + 1, len - i - 1);
                    leftover_size = len - i - 1;
                    return;
                }
            } else if (ch == ' ' || ch == '\t' || ch == '\r')
                continue;
            else
                linebreaks = 0;
        }
        curr_len += len;
    } while (curr_len < size);
}

void parse_head(char *request, char **HTTP_method, char **path, char **ver, char **val) {
    char **head, *line;
    int h_ctr = 0;
    *HTTP_method = strsep(&request, " ");
    *path = strsep(&request, " ");
    *ver = strsep(&request, "\r");

    // get HTTP_method
    if (strcmp(*HTTP_method, "PUT") == 0) {
        head = h_PUT;
        h_ctr = h_PUT_ctr;
    } else if (strcmp(*HTTP_method, "GET") == 0) {
        h_ctr = h_GET_ctr;
        head = h_GET;
    } else {
        printf("Invalid HTTP_method\n");
        return;
    }

    // clear val array
    memset(val, 0, h_ctr * sizeof(val));

    while (1) {
        // ignore leading spaces
        while (*request == '\n' || *request == ' ' || *request == '\t')
            request++;
        line = strsep(&request, "\r");

        if (line == NULL || strlen(line) == 0)    // end of head
            break;

        char *header = strsep(&line, " ");
        char *option = line;

        for (int i = 0; i < h_ctr; i++) {
            if (strcmpi(header, head[i]) == 0) {
                val[i] = option;
                break;
            }
        }
    }

    printf("Method: %s, Path: %s, Version: %s\n", *HTTP_method, *path, *ver);
    for (int i = 0; i < h_ctr; i++) {
        if (val[i] != NULL)
            printf("%s %s\n", head[i], val[i]);
    }

    return;
}

char *content_type_head(char *path) {
    char *ext = strrchr(path, '.');
    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0)
        return "Content-Type: text/html\r\n";
    else if (strcmp(ext, ".txt") == 0)
        return "Content-Type: text/plain\r\n";
    else if (strcmp(ext, ".pdf") == 0)
        return "Content-Type: application/pdf\r\n";
    else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0)
        return "Content-Type: image/jpeg\r\n";
    else if (strcmp(ext, ".mp4") == 0)
        return "Content-Type: video/mp4\r\n";
    // else if (strcmp(ext, ".mp3") == 0)
    //     return "Content-Type: audio/mpeg\r\n";
    // else if (strcmp(ext, ".gif") == 0)
    //     return "Content-Type: image/gif\r\n";
    // else if (strcmp(ext, ".png") == 0)
    //     return "Content-Type: image/png\r\n";
    // else if (strcmp(ext, ".css") == 0)
    //     return "Content-Type: text/css\r\n";
    // else if (strcmp(ext, ".js") == 0)
    //     return "Content-Type: text/javascript\r\n";
    else
        return "Content-Type: text/*\r\n";
}

void send_file(FILE *fp, char *filename, int sockfd) {
    // Send content type
    char *content_type = content_type_head(filename);
    send(sockfd, content_type, strlen(content_type), 0);

    // Send content length
    fseek(fp, 0L, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    char content_length[100];
    sprintf(content_length, "Content-Length: %d\r\n\r\n", size);
    send(sockfd, content_length, strlen(content_length), 0);

    // Send file
    char buffer[BUFFSIZE];
    int sent_size = 0, read_size, total_read_size = 0;
    while ((read_size = fread(buffer, 1, BUFFSIZE, fp)) > 0) {
        total_read_size += read_size;
        sent_size += send(sockfd, buffer, read_size, 0);
    }

    // Interaction statements
    printf("Read %d bytes from local\n", total_read_size);
    printf("Sent %d bytes\n", sent_size);
    printf("File sent successfully: %s\n", filename);
    fflush(stdout);
}

void send_general_response(int status, int sockfd, char *filename) {
    char date[SMALLBUFF];
    time_t now;
    struct tm *timeinfo;
    char *firstline;

    // set first line
    if (status == 200) {
        firstline = "HTTP/1.1 200 OK\r\n";
    } else if (status == 404) {
        firstline = "HTTP/1.1 404 Not Found\r\n";
    } else if (status == 501) {
        firstline = "HTTP/1.1 501 Not Implemented\r\n";
    } else {
        firstline = "HTTP/1.1 500 Internal Server Error\r\n";
    }

    // send first line
    send(sockfd, firstline, strlen(firstline), 0);

    // send date and time
    time(&now);
    timeinfo = localtime(&now);
    timeinfo->tm_mday += 3;
    char *time = asctime(timeinfo);
    time[strlen(time) - 1] = '\0';
    sprintf(date, "Expires: %s\r\n", time);
    send(sockfd, date, strlen(date), 0);

    // send cache control
    char *cache = "Cache-Control: no-store\r\n";
    send(sockfd, cache, strlen(cache), 0);

    // send content language
    char *language = "Content-Language: en-us\r\n";
    send(sockfd, language, strlen(language), 0);

    // send content type
    char *content_type = content_type_head(filename);
    send(sockfd, content_type, strlen(content_type), 0);

    // send server name
    char *server = "Server: MyBrowser\r\n";
    send(sockfd, server, strlen(server), 0);

    // send last modified
    struct stat attr;
    stat(filename, &attr);
    char *last_modified = ctime(&attr.st_mtime);
    last_modified[strlen(last_modified) - 1] = '\0';
    char last_modified_head[100];
    sprintf(last_modified_head, "Last-Modified: %s\r\n", last_modified);
    send(sockfd, last_modified_head, strlen(last_modified_head), 0);
}

void error_404(int sockfd, char *filename) {    // handle 404 error
    send_general_response(404, sockfd, filename);

    FILE *fp = fopen("404.html", "rb");
    send_file(fp, "404.html", sockfd);
    fclose(fp);
}

void error_501(int sockfd, char *filename) {    // handle 501 error
    send_general_response(501, sockfd, filename);

    FILE *fp = fopen("501.html", "rb");
    send_file(fp, "501.html", sockfd);
    fclose(fp);
}

void receive_file_content(FILE *fp, int content_length, int sockfd) {
    printf("Receiving file content...Content length: %d\n", content_length);
    fflush(stdout);

    // if there is some extra data in the buffer, read it
    fwrite(leftover, leftover_size, 1, fp);
    content_length -= leftover_size;
    leftover_size = 0;

    char buffer[BUFFSIZE];
    int read_size, total_read_size = 0;

    // receive file content
    while (content_length > 0) {
        read_size = recv(sockfd, buffer, BUFFSIZE, 0);
        total_read_size += read_size;
        fwrite(buffer, read_size, 1, fp);
        content_length -= read_size;
    }
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno, clilen;
    char buffer[BUFFSIZE];
    struct sockaddr_in serv_addr, cli_addr;    // server and client address
    char *HTTP_method, *path, *ver, *val[max(h_GET_ctr, h_PUT_ctr)];

    if (argc < 2 || (portno = atoi(argv[1])) <= 200) portno = 8080;

    // opens a TCP socket, if successful, returns a descriptor
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed\n");
        exit(1);
    }

    serv_addr.sin_family = AF_INET;            // IPv4
    serv_addr.sin_addr.s_addr = INADDR_ANY;    // localhost
    serv_addr.sin_port = htons(portno);        // port

    // binds the socket to the address and port number specified in serv_addr
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Bind failed\n");
        exit(1);
    }

    // Upto {MAX_CLIENTS} clients can be in the queue
    listen(sockfd, MAX_CLIENTS);
    printf("Server started at port %d...\n", portno);
    fflush(stdout);

    // set home directory to serve files from
    if (getcwd(homedir, sizeof(homedir)) == NULL) {
        perror("Error getting home directory");
        exit(1);
    }

    while (1) {
        printf("\n\nWaiting for new connection...\n\n");
        fflush(stdout);
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0) {
            perror("ERROR on accept\n");
            exit(1);
        } else
            printf("Connected \n");
        fflush(stdout);

        // read request
        recvHTTP(newsockfd, buffer, BUFFSIZE);
        printf("\n\nRequest headers:\n%s\n\n", buffer);
        // printf("\nExtra data:\n%s\n\n", leftover);
        fflush(stdout);
        parse_head(buffer, &HTTP_method, &path, &ver, val);

        // execute request
        if (strcmp(HTTP_method, "PUT") == 0) {
            // put the path in the right format
            char filename[strlen(path) + 2], dirpath[strlen(path) + 2];
            if (filename != NULL && filename[0] != '.') {
                filename[0] = '.';
                strcpy(filename + 1, path);
            }

            // create directory if it doesn't exist
            strcpy(dirpath, filename);
            char *last_slash = strrchr(dirpath, '/');
            *last_slash = '\0';
#ifdef _WIN32
            _mkdir(dirpath);
#else
            mkdir(dirpath, 0777);
#endif

            printf("\n\nReceiving: %s\n\n", filename);
            fflush(stdout);

            FILE *fp = fopen(filename, "wb");

            // file not found
            if (fp == NULL) {
                perror("File not found\n");
                error_404(newsockfd, filename);
                break;
            }

            // receive file content
            int content_length = atoi(val[3]);
            receive_file_content(fp, content_length, newsockfd);

            // close file
            fclose(fp);

            // send general response
            send_general_response(200, newsockfd, filename);
        } else if (strcmp(HTTP_method, "GET") == 0) {
            // put the path in the right format
            char filename[strlen(path) + 2];
            if (filename != NULL && filename[0] != '.') {
                filename[0] = '.';
                strcpy(filename + 1, path);
            }

            printf("\n\nSending: %s\n", filename);
            fflush(stdout);

            FILE *fp = fopen(filename, "rb");

            // file not found
            if (fp == NULL) {
                perror("File not found\n");
                error_404(newsockfd, filename);
                break;
            }

            // send general response
            send_general_response(200, newsockfd, filename);

            // send file
            send_file(fp, filename, newsockfd);

            // close file
            fclose(fp);
        } else {
            error_501(newsockfd, path);
        }

        close(newsockfd);    // close connection
        printf("Disconnected\n");
        fflush(stdout);
    }
    return 0;
}