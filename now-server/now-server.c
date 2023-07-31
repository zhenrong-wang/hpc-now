/*
 * This code is written and maintained by Zhenrong WANG
 * mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * This code is distributed under the license: MIT License
 * Bug report: info@hpc-now.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define DEFAULT_PORT 19885
#define BUFFER_SIZE 1024
#define CMDLINE_LENGTH 2048

int main(int argc, char* argv[]){
    int socket_fd,connect_fd;
    int socket_opt_val=1;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE]="";
    char cmdline[CMDLINE_LENGTH]="";

    if(system("whoami | grep -w hpc-now >> /dev/null 2>&1")!=0){
        printf("[ FATAL: ] Please run this service as the OS user 'hpc-now'.\n");
        return 127;
    }
    if((socket_fd=socket(AF_INET,SOCK_STREAM,0))==-1){
        printf("[ FATAL: ] Failed to create a socket: %s(errno: %d)\n",strerror(errno),errno);
        return 1;
    }
    memset(&server_address,0,sizeof(server_address));
    server_address.sin_family=AF_INET;
    server_address.sin_addr.s_addr=htonl(INADDR_ANY);
    server_address.sin_port=htons(DEFAULT_PORT);
    if(setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,&socket_opt_val,sizeof(socket_opt_val))==-1){
        printf("[ -WARN- ] Failed to set the socket. The service still works.\n");
    }
    if(bind(socket_fd,(struct sockaddr*)&server_address,sizeof(server_address))==-1){
        printf("[ FATAL: ] Failed to bind socket: %s (errno: %d)\n",strerror(errno),errno);
        return 5;
    }
    if(listen(socket_fd,10)==-1){
        printf("[ FATAL: ] Failed to listen socket: %s (errno: %d)\n",strerror(errno),errno);
        return 7;
    }
    while(1){
        if((connect_fd=accept(socket_fd,(struct sockaddr*)NULL,NULL))==-1){
            printf("[ FATAL: ] Failed to accept socket: %s (errno: %d)",strerror(errno),errno);
            continue;
        }
        recv(connect_fd,buffer,BUFFER_SIZE,0);
        sprintf(cmdline,"hpcopr %s",buffer);
        system(cmdline);
        close(connect_fd);
    }
    close(connect_fd);
    close(socket_fd);
}