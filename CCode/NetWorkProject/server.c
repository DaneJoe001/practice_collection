#include<stdio.h>
#include"server.h"

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
    //Initializing every thread identity;
    ThreadInfo threads[10];
    for(int i=0;i<10;i++)
    {
        threads[i].is_used=false;
        threads[i].thread=i+1;
        threads[i].fd_client_socket=-1;
    }

    struct sockaddr_in address_client;
    socklen_t length_client_address=sizeof(address_client);
    //TODO:Add a timer to control the exit of the entire process;
    int index=0;
    bool is_running=switch_execute_status(true,true);

    signal(SIGALRM,alarm_timer);
    alarm(60);
    while(is_running)
    {
        is_running=switch_execute_status(false,false);
        if(threads[index].is_used)
        {
            index++;
            sleep(1);
            continue;
        }
        //Reset index;
        if(index>=10)
        {
            index=0;
        }
        printf("Thread %d is waiting to accept from client!\n",index+1);
        threads[index].fd_client_socket=accept(fd_server_socket,(struct sockaddr*)&address_client,&length_client_address);

        if(threads[index].fd_client_socket<0)
        {
            printf("Failed to accept!\n");
            break;
        }
        threads[index].is_used=true;
        alarm(60);
        int check=pthread_create(&threads[index].thread,NULL,  pthread_handler,&threads[index]);
        if(check!=0)
        {
            printf("Failed to create thread!\n");
            close(threads[index].fd_client_socket);
            threads[index].fd_client_socket=-1;
            break;
        }
    }
    for(int i=0;i<10;i++)
    {
        pthread_join(threads[i].thread,NULL);
    }

    close(fd_server_socket);
    return 0;
}

int get_resouce_fd(uint32_t file_ID,bool operation)
{
    static FileInfo fd_map[COUNT_TOTAL_FILE]={0};
    static bool check_init=false;
    if(!check_init)
    {
        for(int i=0;i<COUNT_TOTAL_FILE;i++)
        {
            fd_map[i].fd=-1;
            memset(fd_map[i].file_path,0,SIZE_BUFFER_PATH);
        }
        strcpy(fd_map[0].file_path,PATH_DEFAULT_RESOURCE);
        check_init=true;
    }
    if(file_ID>COUNT_TOTAL_FILE||file_ID<=0)
    {
        printf("Not found file which have ID %d!\n",file_ID);
        return -1;
    }
    uint32_t file_index=file_ID-1;
    if(operation)
    {
        if(fd_map[file_index].fd<0)
        {
            fd_map[file_index].fd=open(fd_map[file_index].file_path,O_RDONLY);
        }
        if(fd_map[file_index].fd<0)
        {
            printf("Failed to open file which have ID %d!\n",file_ID);
            return -1;
        }
    }
    else
    {
        if(fd_map[file_index].fd>0)
        {
            close(fd_map[file_index].fd);
            fd_map[file_index].fd=-1;
        }
    }
    return fd_map[file_index].fd; 
}

bool switch_execute_status(bool operation,bool new_status)
{
    static bool is_running=false;
    if(operation)
    {
        is_running=new_status;
    }
    return is_running;
}

bool is_block_avalible(Block block)
{
    if(block.offset==-1)
    {
        return false;
    }
    return true;
}

int send_override(int fd_socket, Block block)
{
    if(fd_socket<0)
    {
        printf("Invalied fd_socket!\n");
        return -1;
    }
    if(!is_block_avalible(block))
    {
        printf("Invalied block!\n");
        return -1;
    }
    int fd_resource=get_resouce_fd(block.file_ID,true);
    if(fd_resource<0)
    {
        return -1;
    }
    size_t size_file=lseek(fd_resource,0,SEEK_END);
    printf("File total size: %lu\n",size_file);
    //Check whether the position is within the range of file size;
    if(block.offset>size_file)
    {
        printf("Invalied offset!\n");
        return -1;
    }
    lseek(fd_resource,block.offset,SEEK_SET);
    char buffer_block[SIZE_BLOCK+1]={0};
    printf("Size of block: %lu\n",block.size_block);
    int size_read=read(fd_resource,buffer_block,block.size_block);
    if(size_read<0)
    {
        printf("Failed to read from file %s!\n",PATH_DEFAULT_RESOURCE);
        return -1;
    }
    printf("%d bytes have beed read!\n",size_read);
    //printf("Content :%s\n",buffer_block);
    OriginMessage msg_send;
    msg_send.type=TYPE_SERVER_RESPONSE;
    msg_send.content.server_response.file_ID=block.file_ID;
    msg_send.content.server_response.index_block=block.index_block;
    strncpy(msg_send.content.server_response.buffer,buffer_block,size_read);
    msg_send.content.server_response.size_effective_content=size_read;

    msg_send.length=sizeof(msg_send.type)+sizeof(msg_send.content.server_response.file_ID)+sizeof(msg_send.content.server_response.index_block)+size_read+sizeof(msg_send.content.server_response.size_effective_content);

    //printf("The sended data:\n");
    //print_byte_stream_data(&msg_send);

    char buffer_send[SIZE_BUFFER_CONTENT]={0};

    size_t buffer_length=0;
    bool result =get_byte_stream(buffer_send,&buffer_length,msg_send);
    printf("buffer_length: %lu\n",buffer_length);
    ssize_t size_send=-1;
    if(result)
    {
        size_send=send(fd_socket,buffer_send,buffer_length,0);
    }
    else
    {
        return -1;
    }
    if(size_send<0)
    {
        printf("Failed to send block!\n");
        return -1;
    }
    return size_send;
}

void* pthread_handler(void* thread_info)
{
    if(thread_info==NULL)
    {
        printf("Invalid thread infomation!\n");
        return NULL;
    }
    ThreadInfo* info=(ThreadInfo*)thread_info;

    bool is_finished=false;
    while(!is_finished)
    {
        OriginMessage msg=recv_override(info->fd_client_socket);
        //Check data;
        print_byte_stream_data(&msg);
        if(msg.length==0)
        {
            printf("Failed to recieve the post from client!\n");
        }
        int fd_resource=get_resouce_fd(msg.content.client_post.file_ID, true);
        if(fd_resource<0)
        {
            close(info->fd_client_socket);
            info->fd_client_socket=-1;
            info->is_used=false;

            return NULL;
        }
        Block block=get_block_info(&msg);
        int result = send_override(info->fd_client_socket,block);
        if(result==-1)
        {
            printf("Failed to send infomation!\n");
        }
        if(result<SIZE_BLOCK)
        {
            printf("Finished to send!\n");
            break;
        }
        //Test,send only once;
        //is_finished=true;
    }
    //Clearing file;
    get_resouce_fd(FILE_FIRST,false);
    close(info->fd_client_socket);
    info->fd_client_socket=-1;
    info->is_used=false;
    return NULL;
    
}

void alarm_timer(int signum)
{
    switch_execute_status(true,false);
}