#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/poll.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define MAXLINE 1024
  
int main() { 
    int sockfd; 
    struct sockaddr_in servaddr; 
  
    // Creating socket file descriptor 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(8181); 
    inet_aton("127.0.0.1", &servaddr.sin_addr); 
      
    int n;
    socklen_t len; 
    char *req = "get time", buff[MAXLINE]; 
    len = sizeof(servaddr);

    struct pollfd fds[1];
    int nfds = 1, rc, timeout = 3*1000;

    memset(fds, 0 , sizeof(fds));       // Initialize the pollfd structure  
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;             // Set up the listening socket

    for(int t=1; t<=5; t++) {
        // send request to UDP server to get time
        sendto(sockfd, (const char *)req, strlen(req), 0,
                         (const struct sockaddr *) &servaddr, sizeof(servaddr)); 

        // Call poll() and wait 3 seconds for it to complete
        printf("Try %d: Waiting on poll()...", t);
        rc = poll(fds, nfds, timeout);
       
        if(rc < 0) {               // Check to see if the poll call failed
            perror("\tpoll() failed");
            break;
        }
        else if(rc == 0) {         // Check to see if timeout expired
            printf("\tpoll() timed out.\n");
        }
        else if(fds[0].revents & POLLIN) {
            n = recvfrom(sockfd, (char *)buff, MAXLINE, 0,
                    (struct sockaddr *) &servaddr, &len);
            if(n>0) {
                buff[n] = '\0';
                    // Assumption that received time is already in readable 
                    // date-time format and doesn't need to be processed
                printf("\n\tTime received from server: %s\n", buff); 
                close(sockfd);
                exit(0);
            }
        }
    }

    // if not received in 5 tries
    printf("Timeout exceeded! Shutting down client..\n");
    close(sockfd); 
    return 0; 
} 