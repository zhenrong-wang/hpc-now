/*
 * This code is written and maintained by Zhenrong WANG
 * mailto: zhenrongwang@live.com (*preferred*) | wangzhenrong@hpc-now.com
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * This code is distributed under the license: MIT License
 * Bug report: info@hpc-now.com
 */


/* 
 * Default Port: 19885
 *
 * Socket API Format: 
 * API_CLIENT_EXEC hpcopr_cmd args ...
 * Example: ./myhpcopr graph -c test0001
 * 
 * Server Init Format:
 * SERVER_EXEC (Optional)--client-io (Optional)port_number(10001~65535)
 * Example: ./myserver --client-io 25535
 * 
 * Press Ctrl+C to exit the server.
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
#define SERVER_VERSION_CODE "0.2.0.0001"

int main(int argc, char* argv[]){
    int socket_fd,connect_fd;
    int socket_opt_val=1;
    int port_num=0;
    int i,j;
    struct sockaddr_in server_address;
    char ingress_buffer[BUFFER_SIZE]="";
    char egress_buffer[BUFFER_SIZE]="";
    char cmdline[CMDLINE_LENGTH]="";
    char io_stream[32]="";
    FILE* file_p=NULL;
    
    if(argc==1){
        port_num=DEFAULT_PORT;
        strcpy(io_stream,"local_I/O");
    }
    else if(argc==2){
        if(strcmp(argv[1],"--client-io")==0){
            port_num=DEFAULT_PORT;
            strcpy(io_stream,"client_I/O");
        }
        else{
            strcpy(io_stream,"local_I/O");
            i=atoi(argv[1]);
            if(i>10000&&i<65536){
                port_num=i;
            }
            else{
                port_num=DEFAULT_PORT;
            }
        }
    }
    else{
        if(strcmp(argv[1],"--client-io")==0||strcmp(argv[2],"--client-io")==0){
            strcpy(io_stream,"client_I/O");
        }
        else{
            strcpy(io_stream,"local_I/O");
        }
        i=atoi(argv[1]);
        j=atoi(argv[2]);
        if(i>10000&&i<65536){
            port_num=i;
        }
        else if(j>10000&&j<65536){
            port_num=j;
        }
        else{
            port_num=DEFAULT_PORT;
        }
    }

    printf("[ -INFO- ] Module: HPC-NOW Server (Port: %d) -+- Version: %s\n",port_num,SERVER_VERSION_CODE);
    if(system("whoami | grep -w hpc-now >> /dev/null 2>&1")!=0){
        printf("[ FATAL: ] Please run this service as the OS user 'hpc-now'.\n");
        return 127;
    }
    printf("[ -INFO- ] I/O stream: %s.\n",io_stream);
    if((socket_fd=socket(AF_INET,SOCK_STREAM,0))==-1){
        printf("[ FATAL: ] Failed to create a socket: %s(errno: %d)\n",strerror(errno),errno);
        return 1;
    }
    memset(&server_address,0,sizeof(server_address));
    server_address.sin_family=AF_INET;
    server_address.sin_addr.s_addr=htonl(INADDR_ANY);
    server_address.sin_port=htons(port_num);
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
        if(strcmp(io_stream,"client_I/O")==0){
            dup2(connect_fd,STDIN_FILENO);
            dup2(connect_fd,STDOUT_FILENO);
            dup2(connect_fd,STDERR_FILENO);
        }
        recv(connect_fd,ingress_buffer,BUFFER_SIZE,0);
        sprintf(cmdline,"hpcopr %s",ingress_buffer);
        system(cmdline);
        system("tail -n 1 /usr/.hpc-now/.now-cluster-operation.log > /tmp/now-server-output.tmp 2>&1");
        file_p=fopen("/tmp/now-server-output.tmp","r");
        if(file_p!=NULL){
            fgets(egress_buffer,BUFFER_SIZE,file_p);
            fclose(file_p);
            if(send(connect_fd,egress_buffer,BUFFER_SIZE,0)==-1){
                printf("[ -WARN- ] Failed to send messages to the client.\n");
            }
        }
        else{
            printf("[ -WARN- ] Failed to get the exec results of hpcopr.\n");
        }
        close(connect_fd);
    }
    close(socket_fd);
}