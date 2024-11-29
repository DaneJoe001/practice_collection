#include<iostream>
#include<fstream>
#include<string>

#define PATH_TEST_FILE "./../ServerResource/text.txt"

int main(void)
{
    std::ofstream fout(PATH_TEST_FILE,std::ios::out);
    if(!fout.is_open())
    {
        std::cerr<<"Failed to open file!"<<std::endl;
        return -1;
    }
    std::string end_sign("End\n");
    int begin_char=0;
    for(int i=0;i<50000;i++)
    {
        if(begin_char>=26)
        {
            begin_char=0;
        }
        char ch=begin_char+'a';
        std::string str(1024-end_sign.length(),ch);
        fout<<str<<end_sign;
        begin_char++;
    }
    fout.close();
    return 0;
}