#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<stdbool.h>
#include<string.h>
#include<fcntl.h>

#define PATH_FILE_DEST "./RecievedFile/"
#define SIZE_BUFFER 1024

int main(int argc,const char* argv[])
{
    if(argc!=3)
    {
        printf("Invalied parameter!\n");
        printf("Format: %s [IP] [port]\n",argv[0]);
        return -1;
    }
    int fd_server_socket=socket(AF_INET,SOCK_STREAM,0);
    if(fd_server_socket==-1)
    {
        printf("Failed to get socket!\n");
        return -1;
    }

    struct sockaddr_in address_server;
    address_server.sin_family=AF_INET;
    address_server.sin_port=htons(atoi(argv[2]));
    address_server.sin_addr.s_addr=inet_addr(argv[1]);

    socklen_t length_server_address=sizeof(address_server);

    int check=bind(fd_server_socket,(struct sockaddr*)&address_server,length_server_address);
    if(check<0)
    {
        printf("Failed to bind!\n");
        close(fd_server_socket);
        return -1;
    }
    check=listen(fd_server_socket,16);
    if(check<0)
    {
        perror("Failed to listen!\n");
        close(fd_server_socket);
        return -1;
    }

    struct sockaddr_in address_client;
    socklen_t length_client_address=sizeof(address_client);
    int fd_client_socket=accept(fd_server_socket,(struct sockaddr*)&address_client,&length_client_address);

    if(fd_client_socket<0)
    {
        printf("Failed to accept!\n");
        close(fd_server_socket);
        return -1;
    }


    printf("Please input the dest file name:");
    fflush(stdout);
    char dest_file_name[128]={0};
    scanf("%s",dest_file_name);
    char desr_file_path[256]={0};
    sprintf(desr_file_path,"%s%s",PATH_FILE_DEST,dest_file_name);
    int fd_dest_file=open(desr_file_path,O_WRONLY|O_CREAT|O_TRUNC,0666);
    if(fd_dest_file<0)
    {
        printf("Failed to open dest file!\n");
        close(fd_client_socket);
        close(fd_server_socket);
        return -1;
    }
    char buffer[SIZE_BUFFER]={0};
    int size_recieved=0;
    int size_write=0;
    while(true)
    {
        memset(buffer,0,SIZE_BUFFER);
        size_recieved=recv(fd_client_socket,buffer,SIZE_BUFFER,0);
        if(size_recieved<0)
        {
            printf("Failed to recieved!\n");
            continue;
        }
        else
        {
            size_write=write(fd_dest_file,buffer,size_recieved);
        }
        if(size_write<0)
        {
            printf("Failed to write in file!\n");
            break;
        }
        else if(size_write==0)
        {
            printf("Finished to accept file %s!\n",dest_file_name);
            break;
        }
    }
    close(fd_client_socket);
    close(fd_server_socket);
    close(fd_dest_file);


    return 0;
}