#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <omp.h>
#include <unistd.h>

omp_lock_t lock;
int client_sock[5];
int tid; 
int cli_num = 0;
char *NAME[5];

void send_msg(char* msg, int len){ 
    omp_set_lock(&lock);
    for(int i = 0; i <= cli_num; i++){
       	  send(client_sock[i], msg, len, 0);         
    };
    
    omp_unset_lock(&lock);
}

void recv_cli(int sock_cli){
    int len;
    char msg[10000];
    
    memset(msg, 0, sizeof(msg));

    while(1){
        len = recv(sock_cli, msg, sizeof(msg), 0);
        if(len == 0){
            break;
        }
        send_msg(msg, len);
    }

    printf("%s disconnected....\n",NAME[omp_get_thread_num()]);
    
    
    omp_set_lock(&lock);
    cli_num--;
    omp_unset_lock(&lock);
    
    close(sock_cli);  
}

int main(){
    int sock_ser, sock_cli, cli_len;
    struct sockaddr_in serv, cli;
    
    omp_init_lock(&lock);

    sock_ser = socket(AF_INET, SOCK_STREAM, 0);
    
    if(sock_ser == -1){
        perror("server socket error");
        exit(1);
    }

    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(30030);
    serv.sin_addr.s_addr = inet_addr("0.0.0.0");
    
    if(bind(sock_ser, (struct sockaddr *) &serv, sizeof(serv)) == -1){
        perror("bind error");
        exit(1);
    }
    if(listen(sock_ser, 5) == -1){
        perror("listen error");
        exit(1);
    }
    
    #pragma omp parallel num_threads(5) private(tid, sock_cli)
    {
    	    while(1)
    	    {	
    	   	 tid = omp_get_thread_num();	
           	 cli_len = sizeof(cli);
            	sock_cli = accept(sock_ser, (struct sockaddr *) &cli, (socklen_t *) &cli_len);
            
           	 if(sock_cli == -1){
           	     perror("accept error");
           	     exit(1);
           	 }
            
 	   	char name[10];
 	    	memset(name,0,sizeof(name));
 	    	if(recv(sock_cli, name, sizeof(name), 0) == -1)
            	{
            		perror("recv error");
            		exit(1);
            	}
 	    
 	    	omp_set_lock(&lock);
 	    	cli_num++;    	
            	client_sock[tid] = sock_cli;
            
         	NAME[tid] = name;
            
       	   	omp_unset_lock(&lock);
       	    
            	printf("IP : %s client connected, user name : %s\n", inet_ntoa(cli.sin_addr), NAME[tid]);
            	recv_cli(client_sock[tid]);
            }       
    }
    
    close(sock_ser);
    return 0;
}
