#include<stdio.h>
#include"client.h"

#define SIZE_BUFFER_POST 64

#define PATH_DEFAULT_DOWNLOAD "./ClientDownload/text_download.txt"

//TODO: Check whether the file is exists on the local machine;
int main(int argc,const char* argv[])
{
    int fd_server_socket=socket(AF_INET,SOCK_STREAM,0);
    if(fd_server_socket==-1)
    {
        printf("Failed to socket!\n");
        return -1;
    }
    struct sockaddr_in address_server;
    if(argc!=3)
    {
        //Set default parameters;
        address_server=set_server_addr("127.0.0.1",8888);
    }
    else
    {
        address_server=set_server_addr(argv[1],atoi(argv[2]));
    }

    socklen_t length_server_address=sizeof(address_server);
    int check=connect(fd_server_socket,(struct sockaddr *)&address_server,sizeof(address_server));
    if(check<0)
    {
        printf("Failed to connect!\n");
        close(fd_server_socket);
        return -1;
    }
    printf("Succeeded in establishing a connection!\n");
    //Initializing downloading;
    int fd_download=get_download_file_fd(true);
    if(fd_download<0)
    {
        close(fd_server_socket);
        return -1;
    }
    int size_recv=-1;
    char buffer_post[SIZE_BUFFER_CONTENT]={0};
    char buffer_recv[SIZE_BUFFER_CONTENT]={0};

    uint32_t block_index=0;
    bool is_finished=false;
    while(!is_finished)
    {
        printf("Post block index: %d\n",block_index);
        OriginMessage msg_send;
        msg_send.type=TYPE_CLIENT_POST;
        msg_send.content.client_post.file_ID=FILE_FIRST;
        msg_send.content.client_post.index_block=block_index;
        msg_send.length=sizeof(msg_send.type)+sizeof(msg_send.content.client_post.file_ID)+sizeof(msg_send.content.client_post.index_block)+sizeof(msg_send.length);

        memset(buffer_post,0,SIZE_BUFFER_CONTENT);

        //Check data
        print_byte_stream_data(&msg_send);

        size_t size_buffer=0;
        bool result=get_byte_stream(buffer_post,&size_buffer,msg_send);
        int size_send=send(fd_server_socket,buffer_post,size_buffer,0);
        if(size_send<0)
        {
            printf("Failed to post from server!\n");
            break;
        }

        memset(buffer_recv,0,SIZE_BUFFER_CONTENT);
        size_recv=recv(fd_server_socket,buffer_recv,sizeof(uint32_t),0);
        if(size_recv<0)
        {
            printf("Failed to recieve from server!\n");
            break;
        }

        uint32_t origin_length=0;
        memcpy(&origin_length,buffer_recv,sizeof(uint32_t));
        uint32_t rest_length=ntohl(origin_length);
        //-sizeof(uint32_t);
        printf("Total length:%u\n",ntohl(origin_length));

        size_recv=recv(fd_server_socket,buffer_recv+sizeof(uint32_t),rest_length,0);

        OriginMessage msg_recv=parse_byte_stream(buffer_recv);

        //Check the data;
        //print_byte_stream_data(&msg_recv);
        //printf("%s",msg_recv.content.server_response.buffer);

        Block block=get_block_info(&msg_recv);
        
        lseek(fd_download,block.offset,SEEK_SET);
        int size_write=write(fd_download,msg_recv.content.server_response.buffer,msg_recv.content.server_response.size_effective_content);
        if(size_write<0)
        {
            printf("Failed to write in dest file!\n");
            break;
        }
        if(msg_recv.content.server_response.size_effective_content<SIZE_BLOCK)
        {
            printf("Finished downloading!\n");
            break;
        }
        block_index++;
        //Test,send only once;
        //is_finished=true;
    }
    //Clearing file;
    fd_download=get_download_file_fd(false);
    close(fd_server_socket);
    return 0;
}

int get_download_file_fd(bool operation)
{
    static int fd_download=-1;
    if(operation)
    {
        if(fd_download<0)
        {
            fd_download=open(PATH_DEFAULT_DOWNLOAD,O_RDWR|O_CREAT|O_APPEND, 0666);
        }
        if(fd_download<0)
        {
            printf("Failed to open file %s!\n",PATH_DEFAULT_DOWNLOAD);
            return -1;
        }
    }
    else
    {
        if(fd_download>0)
        {
            close(fd_download);
            fd_download=-1;
        }
    }
    lseek(fd_download,0,SEEK_SET);
    return fd_download; 
}