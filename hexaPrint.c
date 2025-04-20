#include <stdio.h>
#include <stdlib.h>

FILE* f;

void printHex(FILE* buffer, int length)
{
    char* readText = calloc(length,sizeof(char));
    fread(readText,sizeof(char),length,buffer);
    for(int i=0;i<length;i++)
    {
        printf("%.2X ",readText[i]);
    }
}

int main(int argc, char** argv)
{

    f = fopen(argv[1],"r");
    fseek(f,0,SEEK_END);
    long length = ftell(f);
    fseek(f,0,SEEK_SET);
    printHex(f,length);
    fclose(f);
}