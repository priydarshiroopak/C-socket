#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <time.h>       // required for getting system time

#define CHUNK 50         	// max send/ recv size
// #define VERBOSE				// **optional** print input and output

char * recvTCP(int sockfd) {
	// returned string is dynamically allocated and must be freed

	char *input = NULL;
	char tmpbuff[CHUNK+1]={0};
	size_t inputlen = 0, tmplen = 0;

	do {	// do receive calls until empty receive
		tmplen = recv(sockfd, tmpbuff, CHUNK, 0);
		input = realloc(input, inputlen + tmplen + 1);
		strcpy(input + inputlen, tmpbuff);
		inputlen += tmplen;
	} while(tmplen>0 && tmpbuff[tmplen-1]!='\0' && tmpbuff[tmplen-1]!='\n');

	return input;
}

void tostring(unsigned n, char *buff) {
    // store n as string in buff
    if(n) {
        int d=0, l=0, t;
        while(n) {
            buff[d++] = n%10+'0';
            n/=10;
        }
        buff[d]='\0';
        d--;
        while(l<d) {
            t = buff[l];
            buff[l++] = buff[d];
            buff[d--] = t;
        }
    }
    else {
        buff[0]=0, buff[1]='\0';
    }
}

int main(int argc, char **argv) {

    int port;       // check if port value entered by user
    if(argc<2 || (port = atoi(argv[1]))<=2000) {
        perror("Insufficent arguments! Please provide port no for server as command line arg, Exiting...");
        exit(0);
    }
    srand( (unsigned int) time(NULL) % (1+port));  // initialise random number as (unique) seed
    time_t t;              // create time object
    struct tm *tm;

    int sockfd, newsockfd;
    int clilen;
    struct sockaddr_in cli_addr, serv_addr;
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Unable to create socket\n");
        exit(0);
    }

    serv_addr.sin_family        = AF_INET;
    serv_addr.sin_addr.s_addr   = INADDR_ANY;
    serv_addr.sin_port          = htons(port);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) <0) {
        perror("Unable to bind local address\n");
        exit(0);
    }
    else printf("Server running on port %d...\n", port);

    listen(sockfd, 5);
    char buff[100];
    char *req;

    while(1) {

        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if(newsockfd < 0) {
            perror("Accept error\n");
            exit(0);
        }

        req = recvTCP(newsockfd);
        // printf("%s %d\n", req, strlen(req));
        if(strcmp(req, "Send Load")==0) {
            tostring(random()%100+1, buff);         // generate random no (1 t0 100)
            send(newsockfd, buff, strlen(buff)+1, 0);
            printf("Load sent: %s\n", buff);
        }
        else if(strcmp(req, "Send Time")==0) {
            t = time(NULL);
            tm = localtime(&t);                 // initialise time object with local-time
            strftime(buff, sizeof(buff), "%c", tm);    // convert to string to be sent
            // time sent in readable date-time format, no need to proccess on client side
            send(newsockfd, buff, strlen(buff)+1, 0);
            printf("Time sent: %s\n", buff);
        }
        free(req);
        close(newsockfd);
    }
    return 0;
}