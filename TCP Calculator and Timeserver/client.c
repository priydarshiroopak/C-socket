#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CHUNK 7         // max send size

// **optional delay** code to deliberately delay subsequent sent chunks
#include <time.h>
void delay(int s) {
    // Storing start time
    clock_t start = clock();
    // looping till required time is not achieved
    while (clock() < start + 1000*s) ;
}

int main() {
    int sockfd;
    struct sockaddr_in serv_addr;


    int i;
    char buff[100];

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
    else printf("Server connected successfuly!\n");

    int f = 1;          // flag to end loop (1 means continue)

    while(1) {
        printf("Enter arithmetic expression to be evaluated (terminated by newline), or -1 to exit:\n\t");
        char *input = NULL;                   // specify pointer to complete input
        char tmpbuff[CHUNK+1];               // specify buffer for input chunks
        size_t inputlen=0, tmplen = 0;
        while(1) {
            fgets(tmpbuff, CHUNK+1, stdin);  // extract buffer sized data from input stream
            tmplen = strlen(tmpbuff);
            delay(1);                      // **optional** delay may be introduced b/w subsequnt send calls 
            // printf("%s, %d\n", tmpbuff, tmplen);
            if(inputlen<4) {
                input = realloc(input, inputlen + tmplen + 1);
                strcpy(input + inputlen, tmpbuff);
                inputlen += tmplen;
            }
            if(inputlen<4 && strcmp("-1\n", input)==0) {
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
            recv(sockfd, buff, 100, 0);
            printf("Received result\t= %s\n\n", buff);
        }
    }

    // for(int i=0; i<100; i++) buff[i] = '\0';
    // // Assumption that received time is already in readable 
    // // date-time format and doesn't need to be processed

    // if(!close(sockfd)) printf("Server disconnected succesfully\n");
    // else perror("Error closing socket!\n");
    return 0;
}