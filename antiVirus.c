#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* magicNumberStr;

typedef struct virus {
unsigned short SigSize;
unsigned char* VirusName;
unsigned char* Sig;
} virus;


struct link {
struct link *nextVirus;
virus *vir;
};

struct link* vl=NULL;
FILE* checkedFile = NULL;




void printHex(FILE* buffer, int length)
{
    char* readText = calloc(length,sizeof(char));
    fread(readText,sizeof(char),length,buffer);
    for(int i=0;i<length;i++)
    {
        printf("%.2X ",readText[i]);
    }
    free(readText);
}

virus* readVirus(FILE* file)
{
    virus* rVirus=malloc(sizeof(virus));
    char* sigSizeStr = malloc(2);
    unsigned char bEn = '0';
    unsigned char lEn = '0';
    


    fread(sigSizeStr,1,2,file);
    if (feof(file))
    {
        free(rVirus);
        free(sigSizeStr);
        return NULL;
    }
    bEn = sigSizeStr[1];
    lEn = sigSizeStr[0];


    if(memcmp(magicNumberStr,"VIRL",4)==0)
    {
        rVirus->SigSize = (bEn*(0x100))+lEn;
    }
    else if (memcmp(magicNumberStr,"VIRB",4)==0)
    {
        rVirus->SigSize = (lEn*(0x100))+bEn;
    }
    rVirus->Sig = calloc(rVirus->SigSize,1);
    rVirus->VirusName = malloc(16);
    fread(rVirus->VirusName,1,16,file);
    fread(rVirus->Sig,1,rVirus->SigSize,file);
    free(sigSizeStr);
    return rVirus;
}

void printVirus(virus* virus, FILE* output)
{
    fprintf(output,"Virus name:%s\nVirus size:%d\nSignature:\n",virus->VirusName,virus->SigSize);
    for(int i=0;i<virus->SigSize;i++)
    {
        fprintf(output,"%.2X ",virus->Sig[i]);
    }
    fprintf(output,"\n\n");
}

void list_print(struct link *virus_list , FILE* file)
{
    if(virus_list==NULL)
        return;
    printVirus(virus_list->vir,stdout);
    list_print((virus_list->nextVirus),file);
}

struct link* list_append(struct link* virus_list, virus* data)
{
        if(virus_list==NULL)
        {
            virus_list = malloc(sizeof(struct link));
            virus_list->vir = data;
            virus_list->nextVirus = NULL;
        }
        else if(virus_list->nextVirus==NULL)
        {
            struct link* newLink = malloc(sizeof(struct link));
            newLink->vir = data;
            newLink->nextVirus=NULL;
            virus_list->nextVirus=newLink;
        }
        else
        {
            list_append(virus_list->nextVirus,data);
        }
        return virus_list;

        
        
}

void list_free(struct link *virus_list)
{
    if(virus_list==NULL)
        return;
    if(virus_list->nextVirus!=NULL)
        list_free(virus_list->nextVirus);
    free(virus_list->vir->VirusName);
    free(virus_list->vir->Sig);
    free(virus_list->vir);
    free(virus_list);
}

void print_menu()
{
    printf("1) Load signatures\n2) Print signatures\n3) Detect viruses\n4) Fix file\n5) Quit\n");
}

void loadSig()
{
    FILE* file; 
    char* fileName=malloc(100);
    char* input = malloc(100);


    list_free(vl);
    fgets(input,100,stdin);
    sscanf(input,"%99[^\n]",fileName);


    file = fopen(fileName,"r+");
    if(file==NULL)
    {
        free(input);
        free(fileName);
        return;
    }

    magicNumberStr = malloc(4);
    fread(magicNumberStr,1,4,file);
    if ((memcmp(magicNumberStr,"VIRL",4)!=0)&&(memcmp(magicNumberStr,"VIRB",4)!=0))
    {

        fclose(file);
        fprintf(stderr,"Magic number %s is not equal to VIRL or VIRB. exiting\n",magicNumberStr);
        free(magicNumberStr);
        free(input);
        free(fileName);
        return;
    }



    while(!feof(file))
    {
        virus* v = readVirus(file);
        if(v!=NULL)
        {
            vl = list_append(vl,v);
        }

    }

    free(fileName);
    free(input);
    free(magicNumberStr);
    fclose(file);

}

void printSig()
{
    list_print(vl,stdin);
}

void detect_viruses(char* buffer,unsigned int size, struct link* viruses_list)
{
    struct link* currVir = NULL;
    for(int offset=0;offset<size;offset++)
    {
        currVir=viruses_list;
        while(currVir!=NULL)
        {
            if(currVir->vir->SigSize<=size-offset)
            {
                if(memcmp(buffer+offset,currVir->vir->Sig,currVir->vir->SigSize)==0)
                {
                    printf("First byte: %d|%X\nVirus name:%s\nVirus size:%d\n",offset,offset,currVir->vir->VirusName,currVir->vir->SigSize+18);
                }
            }
            currVir=currVir->nextVirus;
        }
    }
}
void neutralize_virus(char *fileName, int signatureOffset)
{
    FILE* write = fopen(fileName,"r+");
    fseek(write,signatureOffset,SEEK_SET);
    char* retCom = malloc(2);
    retCom[0]=0xc3;
    retCom[1]='\0';
    fwrite(retCom,1,2,write);
    fclose(write);
    free(retCom);
}

void fixFile(char* buffer,unsigned int size, struct link* viruses_list, char* fileName)
{
    struct link* currVir = NULL;
    for(int offset=0;offset<size;offset++)
    {
        currVir=viruses_list;
        while(currVir!=NULL)
        {
            if(currVir->vir->SigSize<=size-offset)
            {
                if(memcmp(buffer+offset,currVir->vir->Sig,currVir->vir->SigSize)==0)
                {
                    neutralize_virus(fileName,offset);
                }
            }
            currVir=currVir->nextVirus;
        }
    }
}



int main(int argc, char** argv)
{
    char* buffer = malloc(10000);
    checkedFile = fopen(argv[1],"r+");
    int selected = -1;
    char* readLine = malloc(100);
    int size=10000;

    fseek(checkedFile,0,SEEK_END);
    if(ftell(checkedFile)<10000)
    {
        size=ftell(checkedFile);
    }
    fseek(checkedFile,0,SEEK_SET);
    fread(buffer,1,size,checkedFile);
    fclose(checkedFile);

    while(selected!=5)
    {
        print_menu();
        fgets(readLine,100,stdin);
        if(feof(stdin))
        {
            free(readLine);
            list_free(vl);
            free(buffer);
            exit(0);
        }
        selected=atoi(readLine);
        switch (selected)
        {
        case 1:
            loadSig();
            break;
        case 2:
            printSig();
            break;
        case 3:

            detect_viruses(buffer,size,vl);
            break;

        case 4:
            fixFile(buffer,size,vl,argv[1]);
            checkedFile = fopen(argv[1],"r+");
            fread(buffer,1,size,checkedFile);
            fclose(checkedFile);
            break;
        
        case 5:
            list_free(vl);
            free(buffer);
            break;
        default:
            printf("out of range\n");
            list_free(vl);
            free(buffer);
            break;
        }
        



    }
    free(readLine);




}