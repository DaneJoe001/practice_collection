#ifndef _SERVER_H_
#define _SERVER_H_

#include<signal.h>
#include"base.h"

#define SIZE_BUFFER_SEND 11264
#define SIZE_BUFFER_RECV 64

#define PATH_DEFAULT_RESOURCE "./ServerResource/text.txt"

typedef struct
{
    pthread_t thread;
    bool is_used;
    int fd_client_socket;
}ThreadInfo;

//parameter: 
//operation,true means openning file,false means closing file;
//function:
//Used for managing resource;
int get_resouce_fd(uint32_t file_ID,bool operation);

//parameter:
//operation,true means switching the status,false means keeping;
bool switch_execute_status(bool operation,bool new_status);

//function:
//Override function recv() to meet my requirements;
OriginMessage recv_override(int fd_socket);

bool is_Block_avalible(Block block);

int send_override(int fd_socket, Block block_client);

void* pthread_handler(void* thread_info);

void alarm_timer(int signum);

//bool check_wait_time();

#endif