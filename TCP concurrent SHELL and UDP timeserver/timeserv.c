#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#include <time.h>           // for processing time

#define MAXLINE 1024 


int main() { 
    int sockfd; 
    struct sockaddr_in servaddr, cliaddr; 
      
    // Create socket file descriptor 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    servaddr.sin_family    = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(8181); 
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }

    printf("\nServer Running on port %d....\n", servaddr.sin_port);
  
    int n; 
    socklen_t len;
    char buffer[MAXLINE]; 
 
    len = sizeof(cliaddr);
    time_t t = time(NULL);
    struct tm *tm;          // initialise time object

    while(1) {
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, 
                ( struct sockaddr *) &cliaddr, &len); 
        buffer[n] = '\0';
        printf("%s\n", buffer); 
        if(strcmp(buffer, "get time")==0) {
            tm = localtime(&t);                          // set time object with local-time
            size_t ret = strftime(buffer, sizeof(buffer), "%c", tm);    // convert to string to be sent
            
            // sleep(5);           // testing code (for timeout)
            
            // time sent in readable date-time format, no need to proccess on client side
            sendto(sockfd, (const char *)buffer, strlen(buffer)+1, 0,
                    (const struct sockaddr *)&cliaddr, len);
        }
    }
    return 0; 
} 