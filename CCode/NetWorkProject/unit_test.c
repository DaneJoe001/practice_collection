#include <stdio.h>
#include "base.h"

int main(void)
{
    uint32_t lenth=0;
    data_type type=TYPE_SERVER_RESPONSE;
    Content content;
    content.server_response.file_ID=1;
    content.server_response.index_block=2;
    OriginMessage msg;
    msg.type=type;
    char str_info[]="This is a string used for examing!";
    content.server_response.size_effective_content=strlen(str_info);
    strncpy(content.server_response.buffer,str_info,strlen(str_info));
    msg.length=sizeof(type)+sizeof(content.server_response.file_ID)+sizeof(content.server_response.index_block)+sizeof(content.server_response.size_effective_content)+content.server_response.size_effective_content;
    memcpy(&(msg.content),&content,sizeof(Content));
    //print_byte_stream_data(&msg);

    char* temp_buffer[SIZE_BUFFER_CONTENT*2]={0};
    size_t temp_size=0;
    bool result=(char*)get_byte_stream(temp_buffer,&temp_size,msg);

    if(result)
    {
        printf("Succeed in format conversion!\n");
    }
    else
    {
        printf("Failed to converse format!\n");
        return -1;
    }
    OriginMessage rebuild_msg=parse_byte_stream(temp_buffer);
    printf("temp_size: %lu\n",temp_size);
    print_byte_stream_data(&rebuild_msg);
    return 0;
}