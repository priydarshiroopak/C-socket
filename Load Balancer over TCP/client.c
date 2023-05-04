#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

int main(int argc, char **argv) {

    int port;       // check if port value entered by user
    if(argc<2 || (port = atoi(argv[1]))==0) {
        perror("Insufficent arguments! Please provide port no of server as command line arg, Exiting...");
        exit(0);
    }

    int sockfd;
    struct sockaddr_in serv_addr;
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Unable to create socket\n");
        exit(0);
    }

    serv_addr.sin_family        = AF_INET;
    inet_aton("127.0.0.1", &serv_addr.sin_addr);
    serv_addr.sin_port          = htons(port);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) <0) {
        perror("Unable to connect to server\n");
        exit(0);
    }
    // printf("connected to serv on %s:%d\n", inet_ntoa(serv_addr.sin_addr), port);
    // Assumption that received time is already in readable 
    // date-time format and doesn't need to be processed
    char *buff = recvTCP(sockfd);
    printf("Received time:\t%s\n", buff);
    free(buff);

    close(sockfd);

    return 0;
}