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

#define PATH_FILE_SOURCE "./SourceFile/"
#define SIZE_BUFFER 1024

bool send_file(int fd_socket,const char* source_file_name)
{
    if(fd_socket<0)
    {
        printf("Failed to send!\n");
        return false;
    }
    char source_file_path[512]={0};
    sprintf(source_file_path,"%s%s",PATH_FILE_SOURCE,source_file_name);
    int fd_source_file=open(source_file_path,O_RDONLY);
    if(fd_source_file<0)
    {
        printf("Failed to open source file!\n");
        return false;
    }
    char buffer[SIZE_BUFFER]={0};
    int size_read=0;
    int size_send=0;
    while(true)
    {
        size_read=read(fd_source_file,buffer,SIZE_BUFFER);
        if(size_read<0)
        {
            printf("Failed to read from source file!\n");
            close(fd_source_file);
            return false;
        }
        else if(size_read==0)
        {
            printf("Finished to transport file %s!\n",source_file_name);
            close(fd_source_file);
            return true;
        }
        size_send=send(fd_socket,buffer,size_read,0);
        if(size_send<0)
        {
            printf("Failed to send!\n");
            close(fd_source_file);
            return false;
        }
    }
}

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

    int check=connect(fd_server_socket,(struct sockaddr *)&address_server,sizeof(address_server));
    if(check<0)
    {
        perror("Failed to connect");
        close(fd_server_socket);
        return -1;
    }
    printf("Succeed to establish connection!\n");
    printf("Please input the source file name:");
    fflush(stdout);
    char source_file_name[128]={0};
    scanf("%s",source_file_name);
    bool result=send_file(fd_server_socket,source_file_name);
    if(result)
    {
        printf("Succeed to transport file!\n");
    }
    else
    {
        printf("Failed to transport file!\n");
    }

    close(fd_server_socket);


    return 0;
}