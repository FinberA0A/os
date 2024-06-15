#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#define blocksize 512
#define one_block disk+sizeof(distNode)

char disk[512*1024];

typedef struct distTable{
    int maxlength;   //单个磁块的大小
    int start;      
    bool useFlag;     //是否被使用过
    struct distTable* next;
}distNode;

distNode * distHead;

typedef struct fileTable{  //FCB
    char fileName[10];
    int start;
    int length;
    int maxlength;
    int protect;          //保护码
    char fileKind[3];      //只读/只写/读写三种模式的文件
    struct tm *timeinfo;   //最近一次对文件的更改
    bool openFlag;
    struct fileTable *next;      //指向下一个文件（隐式链接）（如果使用显示链接的话需要FAT(文件分配表)）
}FCB;

//使用链表的方式存储文件，如下只需要存一个文件的首地址
typedef struct user_file_directory{
    struct fileTable *file;      
    int filenum;
}UFD;

//二级目录中首目录使用MFD表示
typedef struct master_file_directory{
    char userName[10];
    char password[10];
    UFD *user;
    struct master_file_directory *next_user;
}MFD;

//*************************************

//任务一：查看各个结构体所占字节数
void check_all_byte(){
    printf("size of int: %ld\n",sizeof(int));
    printf("size of distNode: %ld\n",sizeof(distNode));
    printf("size of fileTableN: %ld\n",sizeof(FCB));
    printf("size of UFD: %ld\n",sizeof(UFD));
    printf("size of MFD: %ld\n",sizeof(MFD));
}
/*
size of int: 4
size of distNode: 24
size of fileTableN: 56
size of UFD: 8
size of MFD: 40
*/

//任务二：使用memcpy函数查看结构体在char数组上的存储
void use_char_array_to_store_struct(){
    if(1){
        distNode dN;
        dN.maxlength = 1000;
        dN.start = 500;
        dN.useFlag = 0;
        dN.next = NULL;
        memcpy(disk,&dN,sizeof(distNode));
    }
    distNode new_dN;
    memcpy(&new_dN,disk,sizeof(distNode));
    printf("new_distNode: \n maxlength:%d\n start:%d\n useFlag:%d\n next:%p\n",
    new_dN.maxlength,new_dN.start,new_dN.useFlag,new_dN.next);
}

//任务三：通过所给出的各个结构体的大小来分配512*1024大小的char数组空间

/*
以下给出一种假设：
磁盘块的大小设为512Byte,则一共可以有1024个磁盘块
所以前48个磁盘块用于存放distNode
第49个磁盘块用于存放MFD(最多可以有16个user)
最大的情况是每一个user都有10个文件
并且每一个文件都可以占用不超过5个磁盘块
成立
*/

//任务四：在char数组上试用distNode中包含的指针*next
void try_using_pointer_in_char_array(){
    for(int i=0; i<1024; i++){ //一共有1024个块
        distNode dN;
        dN.maxlength = 512;
        dN.start = i*blocksize;  //作为一个块起始点的位数
        dN.useFlag = 1;
        dN.next = (distNode*)(disk+(i+1)*sizeof(distNode));
        memcpy(disk+i*sizeof(distNode),&dN,sizeof(distNode));
    }
    //接下来一个一个取出来
    for(int i=0;i<1024;i++){
        distNode new_dN;
        memcpy(&new_dN,disk+i*sizeof(distNode),sizeof(distNode));
        printf("new_distNode: \n maxlength:%d\n start:%d\n useFlag:%d\n next:%p\n",
        new_dN.maxlength,new_dN.start,new_dN.useFlag,new_dN.next);
    }
}

//任务五：直接使用指针访问char数组中的变量
void direct_into_char_array(){
     if(1){
        distNode dN;
        dN.maxlength = 1000;
        dN.start = 500;
        dN.useFlag = 0;
        dN.next = NULL;
        memcpy(disk+sizeof(distNode),&dN,sizeof(distNode));
    }
    distNode* cur = (distNode*)(one_block);
    printf("the cur pointer:%d\n",cur->maxlength);

}

//任务六：在char型数组中输入内容，并成功访问
void use_char_array_to_store_words(){
    printf("请输入内容：\n");
    scanf("%s",disk);
    scanf("%s",disk+5);
    printf("输入的内容为：\n");
    printf("%s\n",disk+5);
}
 
//任务七：使用time库来输出时间
void check_time(){
    FCB temp;
    time_t raw_time;
    time(&raw_time);
    temp.timeinfo=localtime(&raw_time);
    printf("%s",asctime(temp.timeinfo));
}

//任务八：使用'\0'字符填充disk
void init_disk(){
    memset(disk,'\0',512*1024);
    FCB* fcb_node = (FCB*)(disk+50*blocksize);
    if(fcb_node->fileName[0] == '\0'){
        printf("it's empty and i know\n");
    }
    else{
        printf("you should find another way\n");
    }
    return;
}
//*************************************

int main(){
    //check_all_byte();
    //use_char_array_to_store_struct();
    //try_using_pointer_in_char_array();
    //direct_into_char_array();
    //use_char_array_to_store_words();
    //check_time();
    init_disk();
    return 0;
}