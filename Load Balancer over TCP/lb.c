#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/poll.h>
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

int connect_to_serv(int port, struct sockaddr_in *serv_addr) {
    // helper function to create new socket and connect to a server, returns sockfd
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Unable to create socket\n");
        exit(0);
    }
    serv_addr->sin_family        = AF_INET;
    inet_aton("127.0.0.1", &serv_addr->sin_addr);
    serv_addr->sin_port          = htons(port);
    if (connect(sockfd, (struct sockaddr *) serv_addr, sizeof(*serv_addr)) <0) {
        printf("Unable to connect to server %s:%d\n", inet_ntoa(serv_addr->sin_addr), port);
        exit(0);
    }
    return sockfd;
}

void getload(int *serv_port, struct sockaddr_in* serv_addr, int *load, int n) {
    // get load from n servers whose port number and address is passed as array (all arrays must be of size>=n)
    int sockfd;
    for(int i=0; i<n; i++) {
        sockfd = connect_to_serv(serv_port[i], &serv_addr[i]);
        send(sockfd, "Send Load", strlen("Send Load")+1, 0);
        char *buff = recvTCP(sockfd);
        load[i] = atoi(buff);
        printf("\tLoad received from %s:%d - %d\n", inet_ntoa(serv_addr[i].sin_addr), serv_port[i], load[i]);
        free(buff);
        close(sockfd);
    }
}

int argmin(int load[], int n) {
    // helper function to return index of min value in array
    int m=0;
    for(int i=1; i<n; i++) if(load[m]>load[i]) m=i;
    return m;
}

int main(int argc, char **argv) {
    // printf("%d :\n", argc);
    // for(int i=0; i<argc; i++) 
    // printf("%s :\n", argv[i]);
    int sport[2], lbport;       // check if port value entered by user
    if(argc<4 || (lbport = atoi(argv[1]))==0 || (sport[0] = atoi(argv[2]))==0 || (sport[1] = atoi(argv[3]))==0) {
        perror("Insufficent arguments! Please provide port no of load balancer, server 1, server 2 as command line arg, Exiting...\n");
        exit(0);
    }

    int lb_sockfd, newlb_sockfd, serv_sockfd[2];
    struct sockaddr_in cli_addr, lb_addr, serv_addr[2];
    int clilen = sizeof(cli_addr);

    // create socket for client connection
    if ((lb_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Unable to create socket\n");
        exit(0);
    }
    lb_addr.sin_family        = AF_INET;
    lb_addr.sin_addr.s_addr   = INADDR_ANY;
    lb_addr.sin_port          = htons(lbport);

    // bind socket for allowing client connection
    if (bind(lb_sockfd, (struct sockaddr *) &lb_addr, sizeof(lb_addr)) <0) {
        perror("Unable to bind local address\n");
        exit(0);
    }
    else printf("Load balancer running on port %d...\n", lbport);

    struct pollfd fds[1];
    int nfds = 1, rc, timeout = 5*1000;

    memset(fds, 0 , sizeof(fds));       // Initialize the pollfd structure  
    fds[0].fd = lb_sockfd;
    fds[0].events = POLLIN;             // Set up the listening socket (for clients)

    listen(lb_sockfd, 5);

    int load[2];                        // populate load array with initial values
    getload(sport, serv_addr, load, 2);
    time_t t1, t2;
    int elapsedTime=0;                  // keep track of time elapsed (in ms)

    while(1) {
        // start timer
        time(&t1);
        rc = poll(fds, nfds, timeout-elapsedTime);    // call POLL()
        // stop timer
        time(&t2);

        elapsedTime+=difftime(t2, t1)*1000;   // update elapsed time

        // printf("Elapsed time: %d\n", elapsedTime);
        if(rc < 0) {               // Check to see if the poll call failed
            perror("\tpoll() failed");
            break;
        }
        else if(rc == 0) {         // if timeout expired (every 5 seconds)
            printf("Refreshing load values:\n");
            getload(sport, serv_addr, load, 2);   // refresh load from the servers
            elapsedTime=0;
        }
        else if(fds[0].revents & POLLIN) {
            newlb_sockfd = accept(lb_sockfd, (struct sockaddr *) &cli_addr, &clilen);
            if(newlb_sockfd < 0) {
                perror("Accept error\n");
                exit(0);
            }

            // fork a child process to handle client request
            if(fork() == 0) {
                close(lb_sockfd);       // close old sockfd's
                int m=argmin(load, 2);  // choose server with min load

                int serv_sockfd = connect_to_serv(sport[m], &serv_addr[m]);
                // get time from chosen server
                send(serv_sockfd, "Send Time", strlen("Send Time")+1, 0);
                char *buff = recvTCP(serv_sockfd);
                printf("Sending client request to %s:%d\n", inet_ntoa(serv_addr[m].sin_addr), sport[m]);
                send(newlb_sockfd, buff, strlen(buff)+1, 0);
                free(buff);
                
                close(serv_sockfd);
                close(newlb_sockfd);
                exit(0);
            }
            close(newlb_sockfd);
        }
    }
    close(lb_sockfd);
    return 0;
}