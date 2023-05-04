#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CHUNK   10          // max receive size
#define MSTACK  100000      // max stack size
#define MNUM    40          // max number size (chars)

int status;             // 1 -> ok, 0 -> error
int bracket_pos;
// buffer for current number
char numBuf[MNUM];
int numBufSize;

// define operator and operand stack
char operatorStack[MSTACK];
int operatorStack_top;
double operandStack[MSTACK];
int operandStack_top;

// initialise global variables
void eval_initialize(){
    status = 1, bracket_pos = 0;
    memset(numBuf, '\0', sizeof(char)*MNUM);
    numBufSize = 0;
    memset(operatorStack, '\0', sizeof(char)*MSTACK);
    operatorStack_top = -1;
    memset(operandStack, 0, sizeof(double)*MSTACK);
    operandStack_top = -1;
}

double calc(char op, double x, double y){
    switch(op){
        case '+':   return x+y;
        case '-':   return x-y;
        case '*':   return x*y;
        case '/':   if(y!=0) return x/y;        // not handle divide by 0
        default:    return status = 0;          // error handling
    }
}

void eval_update(char*buf, int bufL) {
    for(int i=0; i<bufL; i++) {
        char ch = buf[i];

        // expression ends
        if(ch == '\0' || ch=='\n') {
            if(numBufSize) {
                // push parsed number to operand stack
                operandStack[++operandStack_top] = atof(numBuf);
                memset(numBuf, '\0', sizeof(char)*numBufSize);
                numBufSize = 0;
            }
            int t=0;
            while(t <= operatorStack_top) {
                operandStack[t+1] = calc(operatorStack[t], operandStack[t], operandStack[t+1]);
                t++;
            }
            return;
        }

        // parsing number
        if(ch=='.' || (ch>='0' && ch<='9')) {
            numBuf[numBufSize++] = ch;
        }
        else if(numBufSize>0) {
            // push parsed number to operand stack
            operandStack[++operandStack_top] = atof(numBuf);
            memset(numBuf, '\0', sizeof(char)*numBufSize);
            numBufSize = 0;
            while(operandStack_top>=bracket_pos) {
                operandStack[bracket_pos] = calc(operatorStack[bracket_pos], operandStack[bracket_pos], operandStack[bracket_pos+1]);
                operandStack_top--;
                operatorStack_top--;
            }
        }

        // handle whitespace
        if(ch==' ' || ch=='\t') continue;

        // push operators to operatorStack
        if(ch=='+' || ch=='-' || ch=='*' || ch=='/') {
            if(operandStack_top <= operatorStack_top) operandStack[++operandStack_top] = 0;
            operatorStack[++operatorStack_top] = ch;
        }

        // handle paranthesis
        if(ch=='(') {
            bracket_pos = operatorStack_top+1;
        }
        else if(ch==')') {
            int t=bracket_pos;
            while(t<=operatorStack_top) {
                operandStack[t+1] = calc(operatorStack[t], operandStack[t], operandStack[t+1]);
                t++;
            }
            operatorStack_top = bracket_pos-1;
            operandStack[bracket_pos] = operandStack[operandStack_top];
            operandStack_top = bracket_pos;
            bracket_pos = 0;
        }
    }
}

double eval_result() { return operandStack[operandStack_top];}

int main() {

    /********************************** DEBUG STATEMENTS, ignore */
    // eval_initialize();
    // char tmpbuff[CHUNK+1];
    // size_t inputlen = 0, tmplen = 0;
    // memset(tmpbuff, '\0', sizeof(char)*(CHUNK+1));
    // do {
    //     fgets(tmpbuff, CHUNK+1, stdin);
    //     tmplen = strlen(tmpbuff);
    //     // tmplen = strlen(tmpbuff);
    //     // input = realloc(input, inputlen + tmplen + 1);
    //     // strcpy(input + inputlen, tmpbuff);
    //     // inputlen += tmplen;
    //     printf("%s, %d\n", tmpbuff, tmplen);
    //     // if(tmplen == 0) {
    //     //     if(--emptyRecv_lim == 0) break;
    //     //     continue;
    //     // }
    //     eval_update(tmpbuff, tmplen);
    // } while(tmpbuff[tmplen-1]!='\0' && tmpbuff[tmplen-1]!='\n');
    // for(int i=0; i<MSTACK; i++) printf("%lf ", operandStack[i]); printf("\n");
    // for(int i=0; i<MSTACK; i++) printf("%c ", operatorStack[i]); printf("\n");
    // printf("%lf\n", eval_result());
    /********************************** DEBUG STATEMENTS, ignore */



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
        printf("Waiting for client connection.\n");
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if(newsockfd < 0) {
            perror("Accept error\n");
            exit(0);
        }
        else printf("Client connected succesfully!\n");

        while(1) {
            eval_initialize();
            // char *input = NULL;
            char tmpbuff[CHUNK+1];
            size_t inputlen = 0, tmplen = 0;
            memset(tmpbuff, '\0', sizeof(char)*(CHUNK+1));
            int s=1;
            do {
                tmplen = recv(newsockfd, tmpbuff, CHUNK, 0);
                // tmplen = strlen(tmpbuff);
                // input = realloc(input, inputlen + tmplen + 1);
                // strcpy(input + inputlen, tmpbuff);
                // inputlen += tmplen;
                // printf("%s, %d\n", tmpbuff, tmplen);
                if(tmplen == 0) s=0;
                eval_update(tmpbuff, tmplen);
            } while(tmplen>0 && tmpbuff[tmplen-1]!='\0' && tmpbuff[tmplen-1]!='\n');
            // printf("%s, %d\n", input, inputlen);
            // free(input);
            if(s) {
                char res_str[MNUM];
                if(status) {
                    snprintf(res_str, MNUM, "%lf", eval_result());
                }
                else strcpy(res_str, "Couldn't evaluate, recheck!!\n");
                printf("%s\n", res_str);
                send(newsockfd, res_str, MNUM, 0);
            }
            else break;
        }
        if(!close(newsockfd)) printf("Client disconnected succesfully\n");
        else perror("Error closing socket!\n");
    }

    return 0;
}