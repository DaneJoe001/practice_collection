#include<stdio.h>
#include"base.h"

struct sockaddr_in set_server_addr(const char* addr,int port)
{
    struct sockaddr_in address_server;
    address_server.sin_family=AF_INET;
    address_server.sin_addr.s_addr=inet_addr(addr);
    address_server.sin_port=htons(port);
    return address_server;
}

bool get_byte_stream(void* buffer,size_t* size_buffer,OriginMessage msg)
{
    if(buffer==NULL||size_buffer==NULL)
    {
        printf("Failed to turn the oringin infomation!\n");
        return false;
    }
    short type_temp=msg.type;
    uint32_t length_processed=htonl(msg.length);
    uint16_t type_processed=htons(msg.type);
    int index_buffer=0;
    memcpy(buffer+index_buffer,&length_processed,sizeof(uint32_t));
    index_buffer+=sizeof(uint32_t);
    memcpy(buffer+index_buffer,&type_processed,sizeof(uint16_t));
    index_buffer+=sizeof(uint16_t);
    uint32_t file_ID_processed=0;
    uint32_t index_block_processed=0;
    switch(type_temp)
    {
        case TYPE_CLIENT_POST:
        file_ID_processed=htonl(msg.content.client_post.file_ID);
        memcpy(buffer+index_buffer,&file_ID_processed,sizeof(uint32_t));
        index_buffer+=sizeof(uint32_t);

        index_block_processed=htonl(msg.content.client_post.index_block);
        memcpy(buffer+index_buffer,&index_block_processed,sizeof(uint32_t));
        index_buffer+=sizeof(uint32_t);
        break;
        case TYPE_SERVER_RESPONSE:
        file_ID_processed=htonl(msg.content.server_response.file_ID);
        memcpy(buffer+index_buffer,&file_ID_processed,sizeof(uint32_t));
        index_buffer+=sizeof(uint32_t);

        index_block_processed=htonl(msg.content.server_response.index_block);
        memcpy(buffer+index_buffer,&index_block_processed,sizeof(uint32_t));
        index_buffer+=sizeof(uint32_t);

        uint32_t size_effective_content_processed=htonl(msg.content.server_response.size_effective_content);
        memcpy(buffer+index_buffer,&size_effective_content_processed,sizeof(uint32_t));
        index_buffer+=sizeof(uint32_t);

        memcpy(buffer+index_buffer,&msg.content.server_response.buffer,msg.content.server_response.size_effective_content);
        index_buffer+=msg.content.server_response.size_effective_content;
        break;
        default:
        return false;
        break;
    }
    *size_buffer=index_buffer;
    return true;
}

OriginMessage parse_byte_stream(void* buffer)
{
    OriginMessage result;
    memset(&result,0,sizeof(OriginMessage));
    if(buffer==NULL)
    {
        return result;
    }
    
    int index_buffer=0;
    uint32_t origin_length=0;
    memcpy(&origin_length,buffer+index_buffer,sizeof(uint32_t));
    index_buffer+=sizeof(uint32_t);
    result.length=ntohl(origin_length);

    uint16_t origin_type=0;
    memcpy(&origin_type,buffer+index_buffer,sizeof(uint16_t));
    index_buffer+=sizeof(uint16_t);
    result.type=ntohs(origin_type);
    uint32_t origin_file_ID=0;
    uint32_t origin_index_block=0;

    switch(result.type)
    {
        case TYPE_CLIENT_POST:

        memcpy(&origin_file_ID,buffer+index_buffer,sizeof(uint32_t));
        index_buffer+=sizeof(uint32_t);
        result.content.client_post.file_ID=ntohl(origin_file_ID);

        memcpy(&origin_index_block,buffer+index_buffer,sizeof(uint32_t));
        index_buffer+=sizeof(uint32_t);
        result.content.client_post.index_block=ntohl(origin_index_block);
        break;

        case TYPE_SERVER_RESPONSE:

        memcpy(&origin_file_ID,buffer+index_buffer,sizeof(uint32_t));
        index_buffer+=sizeof(uint32_t);
        result.content.server_response.file_ID=ntohl(origin_file_ID);

        memcpy(&origin_index_block,buffer+index_buffer,sizeof(uint32_t));
        index_buffer+=sizeof(uint32_t);
        result.content.server_response.index_block=ntohl(origin_index_block);

        uint32_t origin_size_effective_content=0;
        memcpy(&origin_size_effective_content,buffer+index_buffer,sizeof(uint32_t));
        index_buffer+=sizeof(uint32_t);
        result.content.server_response.size_effective_content=ntohl(origin_size_effective_content);

        memcpy(result.content.server_response.buffer,buffer+index_buffer,result.content.server_response.size_effective_content);
        index_buffer+=result.content.server_response.size_effective_content;
        break;
        default:
        break;
    }
    return result;

}

void print_byte_stream_data(const OriginMessage* msg)
{

    printf("length: %d\n",msg->length);
    switch(msg->type)
    {
        case TYPE_CLIENT_POST:
        printf("type: Client post\n");
        printf("file_ID: %u\n",msg->content.client_post.file_ID);
        printf("index_block: %u\n",msg->content.client_post.index_block);
        break;
        case TYPE_SERVER_RESPONSE:
        printf("type: Server response\n");
        printf("file_ID: %u\n",msg->content.client_post.file_ID);
        printf("index_block: %u\n",msg->content.client_post.index_block);
        printf("size_effective_content: %u\n",msg->content.server_response.size_effective_content);
        // char buffer_temp[SIZE_BUFFER_CONTENT]={0};
        // strncpy(buffer_temp,msg->content.server_response.buffer,msg->content.server_response.size_effective_content);
        // printf("buffer: %s\n",buffer_temp);
        break;
        default:
        break;
    }
}

OriginMessage recv_override(int fd_socket)
{
    OriginMessage empty_msg;
    memset(&empty_msg,0,sizeof(empty_msg));
    if(fd_socket<0)
    {
        printf("Invalied fd_socket!\n");
        return empty_msg;
    }

    char buffer_recv[SIZE_MESSAGE]={0};

    //Recieve the infomation of length;
    ssize_t check=recv(fd_socket,buffer_recv,sizeof(uint32_t),0);
    if(check<0)
    {
        printf("Failed to recieve info_length!\n");
        return empty_msg;
    }
    if(check==0)
    {
        printf("Connection has been closed!\n");
        return empty_msg;
    }
    uint32_t origin_data=0;
    memcpy(&origin_data,buffer_recv,sizeof(uint32_t));
    int info_length=ntohl(origin_data);

    //Recieve the rest data;
    uint32_t rest_length=info_length-sizeof(uint32_t);
    check=recv(fd_socket,buffer_recv+sizeof(uint32_t),rest_length,0);
    if(check<0)
    {
        printf("Failed to recieve rest_info!\n");
        return empty_msg;
    }
    if(check==0)
    {
        printf("Connection has been closed!\n");
        return empty_msg;
    }

    OriginMessage msg=parse_byte_stream(buffer_recv);
    return msg;
}

Block get_block_info(const OriginMessage* msg)
{
    //Init with invalid data;
    Block block={-1,-1,-1};
    if(msg==NULL)
    {
        return block;
    }
    block.size_block=SIZE_BLOCK;
    switch(msg->type)
    {
        case TYPE_CLIENT_POST:
        block.file_ID=msg->content.client_post.file_ID;
        block.offset=(block.size_block)*(msg->content.client_post.index_block);
        block.index_block=msg->content.client_post.index_block;
        break;
        case TYPE_SERVER_RESPONSE:
        block.file_ID=msg->content.server_response.file_ID;
        block.offset=(block.size_block)*(msg->content.server_response.index_block);
        block.index_block=msg->content.server_response.index_block;
        break;
        default:
        printf("The type of this message is not supported!\n");
        break;
    }
    return block;
}