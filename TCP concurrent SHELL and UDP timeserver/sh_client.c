#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CHUNK 50         // max send/ recv size

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

int main() {
    int sockfd;
    struct sockaddr_in serv_addr;

    char buff[CHUNK];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Unable to create socket\n");
        exit(0);
    }

    serv_addr.sin_family        = AF_INET;
    inet_aton("127.0.0.1", &serv_addr.sin_addr);
    serv_addr.sin_port          = htons(20000);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) <0) {
        perror("Unable to connect to server\n");
        exit(0);
    }
    else printf("Server connected successfuly on port %d\n", serv_addr.sin_port);

    int n, f = 1;          // flag to end loop (1 means continue)

    // receive login instruction:
    char * log = recvTCP(sockfd);
    n = strlen(log);
    printf("%s :\t", log);
    free(log);

    // get username
    fgets(buff, 50, stdin);
    n = strlen(buff);
    buff[n-1]='\0';
    send(sockfd, buff, n, 0);

    // receive result of user verification
    log = recvTCP(sockfd);
    n = strlen(log);
    if(strcmp("FOUND", log)==0) {
        int f=1;
        printf("Username verified with server, proceed:\n");
        while(1) {
            printf("Enter shell command to run on server (pwd, dir, cd):\n\t");

            char *input = NULL;                   // specify pointer to complete input
            char tmpbuff[CHUNK]={0};               // specify buffer for input chunks
            size_t inputlen=0, tmplen = 0;
            while(1) {
                fgets(tmpbuff, CHUNK, stdin);  // extract buffer sized data from input stream
                tmplen = strlen(tmpbuff);
                // printf("%s, %d\n", tmpbuff, tmplen);
                if(inputlen<8) {
                    input = realloc(input, inputlen + tmplen + 1);
                    strcpy(input + inputlen, tmpbuff);
                    inputlen += tmplen;
                }
                if(inputlen<8 && strcmp("exit\n", input)==0) {
                    f=0;
                    break;
                }
                //            else  if(tmplen != CHUNK || tmpbuff[CHUNK-1]=='\n') {
                else  if(tmpbuff[tmplen-1]=='\n') {
                    tmpbuff[tmplen-1]='\0';
                    send(sockfd, tmpbuff, tmplen, 0);
                    break;
                }
                send(sockfd, tmpbuff, tmplen, 0);
            }
            free(input);
            if(!f) {
                printf("Program terminated\n");
                break;
            }
            else {
                char *out = recvTCP(sockfd);
                if(strcmp(out, "$$$$")==0) {
                    printf("Invalid command\n");
                }
                else if(strcmp(out, "####")==0)  {
                    printf("Error in running command\n");
                }
                else {
                    printf("Received result : \n\n%s\n\n", out);
                }
                free(out);
            }
        }
    }
    else if(strcmp("NOT_FOUND", log)==0) {
        printf("Invalid username\n");
    }
    else {
        printf("Invalid response from server. Terminating program!\n");
    }
    free(log);

    if(close(sockfd)!=0) printf("Couldn't terminate connection to server (%d) !\n", sockfd);
    return 0;
}