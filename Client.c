#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <omp.h>
#include <unistd.h>
//add
#include <time.h>

char name[100];
char msg[10000];


void send_msg(int *sock_cli){
    int sock = *sock_cli;
    char nameAndMsg[11000];
    
    time_t current;
    struct tm *t;
    
    while(1){
        fgets(msg, 10000, stdin);
        if(!strcmp(msg,"q\n")){ // input 'q' -> exit
            close(sock);
            exit(0);
        }
         
        current = time(NULL);
        t = localtime(&current);
   
        sprintf(nameAndMsg, "[%d:%d]%s %s", (t->tm_hour + 6) % 24, t->tm_min, name, msg);
        
        if(send(sock, nameAndMsg, strlen(nameAndMsg), 0) != strlen(nameAndMsg)) {
            perror("send error");
        };
   }
}

void recv_msg(int *sock_cli){
    int sock = *sock_cli;
    char nameAndMsg[11000];
    int len;
    while(1){
        memset(nameAndMsg, 0, sizeof(nameAndMsg));
        len = recv(sock, nameAndMsg, sizeof(nameAndMsg), 0);
        if(len == -1){
            perror("receive error");
            exit(0);
        }
        fputs(nameAndMsg, stdout);
    }
}

int main(int argc, char** argv){
    int sock_cli;
    struct sockaddr_in serv;

    if(argc != 2){
        perror("add argument");
        exit(1);
    }
    sprintf(name, "[%s]", argv[1]);
    
    sock_cli = socket(AF_INET, SOCK_STREAM, 0);

    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(30030);
    serv.sin_addr.s_addr = inet_addr("0.0.0.0");

    if(connect(sock_cli, (struct sockaddr *)&serv, sizeof(serv)) == -1){
        perror("connect error");
        exit(1);
    }

    if(send(sock_cli, name, sizeof(name),0) != sizeof(name))
    {
    	perror("send error");
    	return 0;
    }
    
    #pragma omp parallel num_threads(8)
    {
        #pragma omp sections
        {
            #pragma omp section
            {
                send_msg(&sock_cli);
            }
            #pragma omp section
            {
                recv_msg(&sock_cli);
            }
         }
    }
    return 0;
}
