#ifndef _CLIENT_H_
#define _CLIENT_H_

#include"base.h"

//parameter: 
//operation,true means open file,false means close file;
//function:
//Used for managing resource;
int get_download_file_fd(bool operation);

#endif