#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

#include <dirent.h>			// for dir

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

char * evaluate(char * ip, int iplen) {
	// returned string is dynamically allocated and must be freed

	// handling trailing whitespace(s)
	while(iplen>0 && (ip[iplen-1]=='\n' || ip[iplen-1]=='\0' || ip[iplen-1]==' ' || ip[iplen-1]=='\t')) {
		ip[iplen-1] = '\0';
		iplen--;
	}
	int i, j, k;
	// handling leading whitespace(s)
	i=0;
	while(i<iplen && (ip[i]==' ' || ip[i]=='\t')) i++;
	// locating end of first string (command)
	j=i;
	while(j<iplen && !(ip[j]==' ' || ip[j]=='\t')) j++;
	// locating start of second string (argument)
	k=j;
	while(k<iplen && (ip[k]==' ' || ip[k]=='\t')) k++;

	char *ans = NULL;
	// handling 'pwd' call
	if(j-i == 3 && ip[i]=='p' && ip[i+1]=='w' && ip[i+2]=='d') {
		// pwd ignores any additional argument
		// Get the current working directory: 
		long len;
		char *ans;
		len = pathconf(".", _PC_PATH_MAX);
		if ((ans = (char *)malloc((size_t)len)) != NULL && (ans = getcwd(ans, (size_t)len))!=NULL) {
			return ans;
		}
		else {
			perror("Error encountered in getcwd() call");
			ans = malloc(5);
			strcpy(ans, "####");
		}
	}
	// handling 'dir' call
	else if(j-i == 3 && ip[i]=='d' && ip[i+1]=='i' && ip[i+2]=='r') {
		struct dirent *de;  // Pointer to directory entry
		// pointer of DIR type <dirent.h>
		DIR *dr;

		if(k==iplen) dr = opendir(".");
		// dir considers the whole string proceeding 'dir' as single argument
		else dr = opendir(ip+k);

		if (dr == NULL) {
			perror("Error encountered in opendir() call");
			ans = malloc(5);
			strcpy(ans, "####");
		}
		else {
			long tmplen, alen=0;
			ans=NULL;
			while (de = readdir(dr)) {
				//Condition to exclude hidden/ backup file / directories
				if (de->d_name[0] != '.' && de->d_name[strlen(de->d_name)-1] != '~') {
					tmplen = strlen(de->d_name);
					ans = realloc(ans, alen + tmplen + 2);
					strcpy(ans + alen, de->d_name);
					alen += tmplen+1;
					ans[alen-1] = '\n';
					ans[alen] = '\0';
					// printf("%s\n",de->d_name);
				}
			}
			closedir(dr);
		}
	}
	// handling 'cd' call
	else if(j-i==2 && ip[i]=='c' && ip[i+1]=='d') {
		if(k!=iplen && chdir(ip+k)==0) {
			ans = malloc(20);
			strcpy(ans, "Directory changed!");
		}
		else {
			perror("Error encountered in chdir() call");
			ans = malloc(5);
			strcpy(ans, "####");
		}
	}
	// handling invalid command
	else {
		ans = malloc(5);
		strcpy(ans, "$$$$");
	}
	return ans;
}

int main()
{
	/*******DEBUG statements -- IGNORE*********************************/
	// char st[200];
	// fgets(st, 200, stdin);
	// st[strlen(st)-1] = '\0';
	// printf("command \t: %s\n, %d", st, strlen(st));
	// char * ans = evaluate(st, strlen(st));
	// printf("Output \t: %s\n", ans);
	// free(ans);
	/*******DEBUG statements -- IGNORE*********************************/



	int	sockfd, newsockfd ; // Socket descriptors for parent-child processes
	int	clilen;
	struct sockaddr_in	cli_addr, serv_addr;

	int n;
	char buff[CHUNK];

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(20000);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("Unable to bind local address\n");
		exit(0);
	}
	else {
		printf("Server Running on port (%d)....\n", serv_addr.sin_port);
	}

	listen(sockfd, 5);

	while (1) {

		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen) ;

		if (newsockfd < 0) {
			perror("Accept error!\n");
			exit(0);
		}
		else {
			printf("Client connected succesfully on port %d\n", cli_addr.sin_port);
		}

		// server forks, parent skips this if and continues to wait on accept
		if (fork() == 0) {
			// Child process communicates with client	
			if(close(sockfd)!=0) printf("Couldn't terminate connection to client (%d) !\n", cli_addr.sin_port); // close old socket
			
			strcpy(buff,"LOGIN");
			send(newsockfd, buff, strlen(buff) + 1, 0);

			char * rcvd_uname = recvTCP(newsockfd);
			n = strlen(rcvd_uname);

		#ifdef VERBOSE
			printf("Received username from client(%d): %s\n", cli_addr.sin_port, rcvd_uname);
		#endif

			FILE* fptr;
			char uname[50];
			fptr = fopen("users.txt", "a+");
		
			if (!fptr) {
				perror("Error encountered opening username file!\n");
				if(close(newsockfd)!=0) printf("Couldn't terminate connection to port (%d) !\n", cli_addr.sin_port);
				exit(0);
			}
		
			int f=0;
			while (fgets(uname, 50, fptr) != NULL) {
				int l = strlen(uname);
				if(l>0 && uname[l-1]=='\n') uname[--l]='\0';
				if(strcmp(uname, rcvd_uname)==0) {
					f=1;
					break;
				}
			}
			free(rcvd_uname);
			fclose(fptr);

			if(!f) {	// username not found
				strcpy(buff, "NOT_FOUND");
				send(newsockfd, buff, strlen(buff) + 1, 0);
			}
			else {		// username found
				strcpy(buff, "FOUND");
				send(newsockfd, buff, strlen(buff) + 1, 0);
				while(1) {
					// receive commands from client
					char *input = recvTCP(newsockfd);
					int inputlen = strlen(input);

				#ifdef VERBOSE
					printf("Command from client(%d) : %s\n", cli_addr.sin_port, input);
				#endif
					
					if(inputlen) {		// execute and return if command received
						char *ans = evaluate(input, inputlen);
						int alen = strlen(ans)+1, sent=0, lim;

					#ifdef VERBOSE
						printf("Reply from server to client(%d) : %s\n", cli_addr.sin_port, ans);
					#endif

						while(sent < alen) {	// send areply in chunks
							lim = (sent+CHUNK < alen)?(sent+CHUNK):alen;
							send(newsockfd, ans+sent, lim-sent, 0);
							sent = lim;
						}
						free(ans);
						free(input);
					}
					else {
						free(input);
						break;
					}
				}
			}

			if(close(newsockfd)!=0) printf("Couldn't terminate connection to client (%d) !\n", cli_addr.sin_port);
			else printf("Connection to client terminated successfuly on port %d\n", cli_addr.sin_port);
			exit(0);
		}

		if(close(newsockfd)!=0) printf("Couldn't terminate connection to client (%d) !\n", cli_addr.sin_port);
	}
	return 0;
}