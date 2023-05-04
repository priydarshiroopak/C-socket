#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <time.h>       // required for getting system time

int main() {
    int sockfd, newsockfd;
    int clilen;
    struct sockaddr_in cli_addr, serv_addr;
    
    int i;
    char buff[100];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Unable to create socket\n");
        exit(0);
    }

    serv_addr.sin_family        = AF_INET;
    serv_addr.sin_addr.s_addr   = INADDR_ANY;
    serv_addr.sin_port          = htons(20000);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) <0) {
        perror("Unable to bind local address\n");
        exit(0);
    }

    listen(sockfd, 5);

    while(1) {

        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if(newsockfd < 0) {
            perror("Accept error\n");
            exit(0);
        }
        
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);                          // initialise time object with local-time
        size_t ret = strftime(buff, sizeof(buff), "%c", tm);    // convert to string to be sent

        // time sent in readable date-time format, no need to proccess on client side
        send(newsockfd, buff, strlen(buff)+1, 0);

        close(newsockfd);
    }
    return 0;
}