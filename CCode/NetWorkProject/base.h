#ifndef _BASE_H_
#define _BASE_H_

#include<netinet/in.h>
#include <arpa/inet.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<stdbool.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<pthread.h>

#define TYPE_CLIENT_POST 1
#define TYPE_SERVER_RESPONSE 2

#define SIZE_BUFFER_CONTENT 10240
#define SIZE_BUFFER_PATH 1024
#define SIZE_MESSAGE 11264
#define SIZE_BLOCK 1024

#define FILE_FIRST 1

#define COUNT_TOTAL_FILE 1
typedef uint16_t data_type;

typedef struct
{
    //file_ID is the file number that starts from one.
    uint32_t file_ID;
    uint32_t index_block;
}ClientPost;

typedef struct
{
    uint32_t file_ID;
    uint32_t index_block;
    uint32_t size_effective_content;
    char buffer[SIZE_BUFFER_CONTENT];
}ServerResponse;

typedef union
{
    ClientPost client_post;
    ServerResponse server_response;

}Content;

typedef struct
{
    uint32_t length;
    data_type type;
    Content content;

}OriginMessage;

typedef struct
{
    uint32_t file_ID;
    uint32_t index_block;
    size_t offset;
    size_t size_block;
}Block;

typedef struct
{
    int fd;
    char file_path[SIZE_BUFFER_PATH];
}FileInfo;

struct sockaddr_in set_server_addr(const char* addr,int port);

//Parameter:
//buffer is used for storage the dest infomation;
//size_buffer is used for getting the real size of infomation;
//msg means the origin message which is used to be turned;
//function:
//Generate byte stream;
bool get_byte_stream(void* buffer,size_t* size_buffer,OriginMessage msg);

//Returned value:
//invalid result will be filled with 0
//function:
//Subtract data from buffer;
OriginMessage parse_byte_stream(void* buffer);

void print_byte_stream_data(const OriginMessage* msg);

OriginMessage recv_override(int fd_socket);

Block get_block_info(const OriginMessage* msg);

#endif