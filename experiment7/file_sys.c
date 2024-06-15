#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#define MaxUser 100
#define MaxDisk 512*1024  //表示磁盘的大小 为512KB
#define commandAmount 12
#define Blocksize 512
#define MFD_Start disk+48*Blocksize
#define UFD_Start disk+49*Blocksize
#define FCB_Start disk+50*Blocksize
#define Text_Start disk+65*Blocksize

char disk[MaxDisk];//为了使磁盘的模拟更为贴切，将distNode/fileTableN/UFD/MFD/userTable都存放在disk数组中
time_t raw_time;
char* readable = "r";  //filekind
char* writeable = "w";
char* rwable = "rw";
/*由计算可知以下各数据所占字节数：
distNode 24 Byte
FCB(fileTable) 56 Byte
UFD 16 Byte
MFD 40 Byte 
*/

/*disk存放规划：
以下给出一种假设：
磁盘块的大小设为512Byte,则一共可以有1024个磁盘块
所以前48个磁盘块用于存放distNode
第49个磁盘块用于存放MFD(最多可以有12个user)
最大的情况是每一个user都有10个文件
并且每一个文件都可以占用不超过6个磁盘块

以下为磁盘块具体分配情况
    1-48    |   49   |   50   |   51-65   |   66-1024
  distNode     MFDs     UFDs       FCBs        text
*/

/*struct tm中的数据情况：
struct tm {
   
    int tm_sec;   // 秒，范围从 0 到 59
    int tm_min;   // 分，范围从 0 到 59
    int tm_hour;  // 时，范围从 0 到 23
    int tm_mday;  // 一个月中的日，范围从 1 到 31
    int tm_mon;   // 月份，范围从 0 到 11
    int tm_year;  // 年份，从 1900 开始
    int tm_wday;  // 一周中的日，范围从 0 (周日) 到 6 (周六)
    int tm_yday;  // 一年中的日，范围从 0 到 365
    int tm_isdst; // 夏令时标识
};

*/

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
    FCB *next=NULL;      //指向下一个文件（隐式链接）（如果使用显示链接的话需要FAT(文件分配表)）
}FCB;

//使用链表的方式存储文件，如下只需要存一个文件的首地址
typedef struct user_file_directory{
    FCB *file;  
    int filenum=0;    
}UFD;

//二级目录中首目录使用MFD表示
typedef struct master_file_directory{
    char userName[10];
    char password[10];
    UFD *user;
    MFD *next_user;
}MFD;

MFD* userHead;
int used = 0;   //用户数（因为对磁盘的模拟也就是一个全局空间的数组，所以used声明为全局变量可行）
int using = 0;   //当前用户被创建时的编号
MFD* current_user = NULL;//作为当前用户的指针

void fileCreate(char fileName[], int length, char fileKind[], int Protect);

void fileWrite(char fileName[],int protect);

void fileCat(char fileName[],int protect);           //展示文件的内容

void fileRen(char fileName[],char rename[],int protect);  // 文件重命名

void fileFine(char fileName[]);               //文件查询

void fileDir();           //展示当前用户的所有文件

void fileClose(char fileName[],int protect);

void fileDel(char fileName[],int protect);

void chmod(char fileName[], char kind[],int protect);  //更改文件的模式

int requestDist(int startPosition, int maxlength);  //展示磁盘分配情况

void initDisk();                     //初始化磁盘(将一些必须的空间分配好，如MFD)

void freeDisk(int startPosition);     //从某位置开始释放磁盘空间

void diskShow();                      //展示磁盘的使用情况

void userCreate();

int login();

//初始化磁盘(将一些必须的空间分配好，如MFD)
void initDisk(){
    //首先要将前48个磁盘块用distNode进行初始化
    memset(disk,'\0',MaxDisk);
    distNode dN;
    //每一个磁盘块大小为512Byte，所以应该有1024个distNode分别进行描述
    for(int i=0; i<1024; i++){
        dN.maxlength = 512;
        dN.start = i*Blocksize;
        dN.useFlag = 1:0?i<48; //在还没有创建用户时只有distNode存放的被使用
        dN.next = (distNode*)(disk + (i+1)*sizeof(distNode));
        memcpy(disk+i*sizeof(distNode),&dN,sizeof(dN));
    }
}                     

void userCreate(){
    //首先判断是否是第一个用户
    if(used == 0){
        //要将描述磁盘的distNode进行更新

        distNode* node49;
        node49 = MFD_Start;
        node49->useFlag = 1;

        //使用给出的userhead作为首节点
        userHead = (MFD*)malloc(sizeof(MFD));
        printf("please enter username:");
        scanf("%s",userHead->userName);
        printf("please enter the password:");
        scanf("%s",userHead->password);
        //此时该用户名下没有文件
        userHead->user = NULL;
        //此时也没有后续其他用户
        userHead->next_user = NULL;

        //将这第一个用户的信息存入第49个磁盘块中
        memcpy(MFD_Start,userHead,sizeof(MFD));
        free(userHead);
        userHead = (MFD*)(MFD_Start);
        used++;
        return;
    }
    //若不是第一个用户
    MFD new_user;
    //使用used来更新前一个用户的next_user参数
    MFD* cur_user=userHead;
    while(cur_user->next_user != NULL){
        cur_user = cur_user->next_user;
    }
    cur_user->next_user = (MFD*)(MFD_Start+used*sizeof(MFD));

    //使用给出的userhead作为首节点
    printf("please enter username:");
    scanf("%s",new_user.userName);
    printf("please enter the password:");
    scanf("%s",new_user.password);
    //此时该用户名下没有文件
    new_user.user = NULL;
    memcpy(MFD_Start+used*sizeof(MFD),&new_user,sizeof(MFD));
    used++;    
    return;
}

//输入账号和密码，返回1/0表示登陆成功或失败
int login(){
    //flag作为最后的返回值
    int flag=0;
    char name[10];
    char pwd[10];
    printf("please enter the user's name:");
    scanf("%s",name);
    printf("please enter the password:");
    scanf("%s",pwd);

    //从userHead直接读取MFD进行比较
    //若有所找用户且密码正确，则将其地址赋给current_user指针

    MFD* cur_user = userHead;
    while(cur_user->next_user!=NULL){
        using++;
        //比较name和pwd
        if(!strcmp(cur_user->userName,name)){
            if(!strcmp(cur_user->password,pwd)){
                flag = 1;
                current_user = cur_user;
                break;
            }
        }
        cur_user = cur_user->next_user;
    }
    return flag;

    //maybe write prompt to create new user

    /*
    compliment
    */
}

//创建文件，输入文件名，文件长度，和文件类型(已知current_user))以及保护码
void fileCreate(char fileName[], char fileKind[], int Protect){

    distNode* node50;
    //若该用户没有创建过文件
    if(current_user->user == NULL){
        //首先更改distNode
        
        node50 = (distNode*)(disk+49*sizeof(distNode));
        node50->useFlag = 1;
        //然后根据used数值在current_user更改MFD中的user变量的值
        current_user->user = (UFD*)(UFD_Start+(using-1)*sizeof(UFD)); 

        //然后在UFDs中存入相应的根据used计算所得FCB的地址
        //used个用户，*10个文件，*56位空间
        current_user->user->file = (FCB*)(FCB_Start+(using-1)*sizeof(FCB)*10);

        FCB* cur_file = current_user->user->file;
        current_user->user->filenum++;

        //输入创建的文件的各类信息
        strcpy(cur_file->fileName,fileName);
        cur_file->length = 0;
        strcpy(cur_file->fileKind,fileKind);
        cur_file->maxlength = Blocksize*6; //最多占用6个磁盘块
        cur_file->next = NULL;
        cur_file->openFlag = 0;
        cur_file->protect = Protect; //需要用户进行输入protect码
        cur_file->start = 66*Blocksize+(using-1)*10*Blocksize*6;    //以disk为逻辑0单位
        cur_file->timeinfo = localtime(&raw_time);

        //更改记录FCB的磁盘块的distNode
        int fcb_block = ceil((50*Blocksize+(using-1)*sizeof(FCB)*10)/Blocksize) ;
        //继续使用node50这一变量
        node50 = (distNode*)(disk+fcb_block*sizeof(distNode));
        node50->useFlag = 1;
        return;
    }

    FCB* cur_file = current_user->user->file;
    current_user->user->filenum++;
    while(cur_file->next!=NULL){
        cur_file = cur_file->next;
    }
    cur_file->next = (FCB*)(FCB_Start+(using-1)*10*sizeof(FCB)+(current_user->user->filenum-1)*sizeof(FCB));
    //该用户已有文件
    FCB new_fcb;
    //输入创建的文件的各类信息
    strcpy(new_fcb.fileName,fileName);
    new_fcb.length = 0;
    strcpy(new_fcb.fileKind,fileKind);
    new_fcb.maxlength = Blocksize*6; //最多占用6个磁盘块
    new_fcb.next = NULL;
    new_fcb.openFlag = 0;
    new_fcb.protect = Protect; //需要用户进行输入protect码
    new_fcb.start = 66*Blocksize+(using-1)*10*Blocksize*6+(current_user->user->filenum-1)*Blocksize*6;    //以disk为逻辑0单位
    new_fcb.timeinfo = localtime(&raw_time);
    memcpy(FCB_Start+(using-1)*10*sizeof(FCB)+(current_user->user->filenum-1)*sizeof(FCB),&new_fcb,sizeof(FCB));

    //更改记录FCB的磁盘块的distNode
    int fcb_block = ceil(((50*Blocksize+(using-1)*sizeof(FCB)*10)+(current_user->user->filenum-1)*sizeof(FCB))/Blocksize) ;
    //继续使用node50这一变量
    node50 = (distNode*)(disk+fcb_block*sizeof(distNode));
    node50->useFlag = 1;
    return;

}

//写文件，在其中判断是否可写
void fileWrite(char fileName[],int length,int protect){
    FCB* current_file = current_user->user->file;
    while(current_file->next!=NULL){
        //首先要找到当前名字文件
        if(!strcmp(current_file->fileName,fileName)){
            break;
        }
        current_file = current_file->next;
    }
    //检查保护码
    if(current_file->protect != protect){
        printf("保护码错误\n");
        return;
    }
    //如果文件不可写
    if(!strcmp(current_file->fileKind,readable)){
        printf("该文件不可写\n");
    }
    else{
        current_file->openFlag = 1;
        current_file->length = length;
        printf("本文件可写，请在以下输入内容：\n");
        //写入文件内容,需要找到文本距离在磁盘块中的位置
        scanf("%s",disk+current_file->start);
        //在这里可以使用输入限制长度，但这里先使用scanf
        //以下使用length判断是否过长导致的多个盘块被使用

        //更新FCB
        current_file->timeinfo = localtime(&raw_time);

        //更新记录text的distNode
        int text_block_floor = ceil(current_file->start/Blocksize);
        int text_block_ceil = ceil((current_file->start+length)/Blocksize);
        while(text_block_floor<=text_block_ceil){
            distNode* dn = (distNode*)(disk+text_block_floor*sizeof(distNode));
            dn->useFlag = 1;     
            text_block_floor++;       
        }
    }
}

//展示文件的内容
void fileCat(char fileName[],int protect){
    FCB* current_file = current_user->user->file;
    while(current_file->next!=NULL){
        //首先要找到当前名字文件
        if(!strcmp(current_file->fileName,fileName)){
            break;
        }
        current_file = current_file->next;
    }
    //检查保护码
    if(current_file->protect != protect){
        printf("保护码错误\n");
        return;
    }
    //如果文件不可读
    if(!strcmp(current_file->fileKind,writeable)){
        printf("该文件不可读");
        return;
    }
    //设定一个char*数组，将dist数组中的数值拷贝出来
    current_file->openFlag = 1;
    char context[6*Blocksize];
    memcpy(context,disk+current_file->start,current_file->length);
    printf("文件中的内容如下：\n %s",context);
    return;
}        

// 文件重命名
void fileRen(char fileName[],char rename[],int protect){
    FCB* current_file = current_user->user->file;
    while(current_file->next!=NULL){
        //首先要找到当前名字文件
        if(!strcmp(current_file->fileName,fileName)){
            break;
        }
        current_file = current_file->next;
    }
    //检查保护码
    if(current_file->protect != protect){
        printf("保护码错误\n");
        return;
    }
    //在fcb中更改文件名字
    strcpy(current_file->fileName,rename);

    //更新FCB
    current_file->timeinfo = localtime(&raw_time);
}  

//文件查询
void fileFine(char fileName[]){
    //找到该文件后将FCB中的信息展示
    FCB* current_file = current_user->user->file;
    while(current_file->next!=NULL){
        //首先要找到当前名字文件
        if(!strcmp(current_file->fileName,fileName)){
            break;
        }
        current_file = current_file->next;
    } 
    //将需要的位置的fcb临时的fcb中
    FCB temp_fcb;   
    memcpy(&temp_fcb,current_file,sizeof(FCB));
    //打印文件的各个信息
    printf("当前文件的信息：\n
    文件名：%s\n
    文件长度：%d\n
    最大长度：%d\n
    修改日期：%s\n
    保护码：%d\n
    文件类型：%s\n
    ",temp_fcb.fileName,temp_fcb.length,temp_fcb.maxlength,asctime(temp_fcb.timeinfo),temp_fcb.protect,temp_fcb.fileKind)
}   

//展示当前用户的所有文件(列文件目录 name protect address length)
void fileDir(){
    //current_user 表示当前用户，第一个文件的地址存在UFD中，所以声明一个变量存放文件的物理地址
    FCB* current_file = current_user->user->file;
    printf("  文件名   |   保护码   |    物理地址    |    文件类型    |    文件长度    |")；
    while(current_file->next!=NULL){
        //打印文件内容
        printf("  %s   %d   %p   %s    %d\n",current_file->fileName,current_file->protect,
        current_file, current_file->fileKind, current_file->length);
        current_file = current_file->next;
    }
}   

//关闭打开的文件
void fileClose(char fileName[],int protect){
    //找到该文件后将FCB中的信息展示
    FCB* current_file = current_user->user->file;
    while(current_file->next!=NULL){
        //首先要找到当前名字文件
        if(!strcmp(current_file->fileName,fileName)){
            break;
        }
        current_file = current_file->next;
    } 
    //检查保护码
    if(current_file->protect != protect){
        printf("保护码错误\n");
        return;
    }
    current_file->openFlag = 0;
}

//将文件删除
void fileDel(char fileName[],int protect){
    FCB* former_FCB;
    FCB* current_file = current_user->user->file;
    while(current_file->next!=NULL){
        //首先要找到当前名字文件
        if(!strcmp(current_file->fileName,fileName)){
            break;
        }
        former_FCB = current_file;
        current_file = current_file->next;
    }
    //检查保护码
    if(current_file->protect != protect){
        printf("保护码错误\n");
        return;
    }

    //更改distNode中对context的描述
    //更新记录text的distNode
    int text_block_floor = ceil(current_file->start/Blocksize);
    int text_block_ceil = ceil((current_file->start+length)/Blocksize);
    while(text_block_floor<=text_block_ceil){
        distNode* dn = (distNode*)(disk+text_block_floor*sizeof(distNode));
        dn->useFlag = 0;     
        text_block_floor++;       
    }

    //更改FCB中的内容
    current_file->start = 0;
    current_file->length = 0;
    current_file->maxlength = 0;
    current_file->openFlag = 0;
    current_file->protect = 0;
    memset(current_file->fileName,'\0',sizeof(current_file->fileName));
    memset(current_file->fileKind,'\0',sizeof(current_file->fileKind));
    //更改上一个FCB的next
    former_FCB->next = current_file->next->next;

    //更改distNode中的对fcb的描述
    //更改记录FCB的磁盘块的distNode
    int fcb_block = ceil(((50*Blocksize+(using-1)*sizeof(FCB)*10)+(current_user->user->filenum-1)*sizeof(FCB))/Blocksize) ;
    //继续使用node50这一变量
    distNode* node;
    node = (distNode*)(disk+fcb_block*sizeof(distNode));
    node->useFlag = 0;
    return;
}

//更改文件的模式
void chmod(char fileName[], char kind[],int protect){
    FCB* current_file = current_user->user->file;
    while(current_file->next!=NULL){
        //首先要找到当前名字文件
        if(!strcmp(current_file->fileName,fileName)){
            break;
        }
        current_file = current_file->next;
    }
    //检查保护码
    if(current_file->protect != protect){
        printf("保护码错误\n");
        return;
    }
    strcpy(current_file->fileKind,kind);
    current_file->timeinfo = localtime(&raw_time);
    return;
}

/*
以下为磁盘块具体分配情况
    1-48    |   49   |   50   |   51-65   |   66-1024
  distNode     MFDs     UFDs       FCBs        text

distNode 24 Byte
FCB(fileTable) 56 Byte
UFD 16 Byte
MFD 40 Byte 
*/

//磁盘分配查询
//maxlength取值24/56/16/40
void requestDist(int startPosition, int maxlength){
    //根据以上磁盘
    int dist_MFD_boundary = 48*Blocksize;
    int MFD_UFD_boundary = 49*Blocksize;
    int UFD_FCB_boundary = 50*Blocksize;
    int FCB_text_boundary = 65*Blocksize;
    int block_num = startPosition/Blocksize;
    if(block_num<48){//当前查看的是distNode
        //distNode* request_node;
        //request_node = disk+block_num*Blocksize;
        int i = startPosition/sizeof(distNode);
        i++;
        printf("该位置记录第%d块磁盘块的数据\n",i);
    }
    else if(block_num<49){
        int i = (startPosition-dist_MFD_boundary)/sizeof(MFD);
        i++;
        printf("该位置记录第%d个MFD\n",i);
    }
    else if(block_num<50){
        int i = (startPosition-MFD_UFD_boundary)/sizeof(UFD);
        i++;
        printf("该位置记录第%d个UFD\n",i);
    }
    else if(block_num<65){
        printf("该位置记录FCB\n");
    }
    else if(block_num<1024){
        printf("该位置记录具体文本\n");
        char* context = (char*)malloc(maxlength);
        memcpy(context,disk+startPosition,maxlength);
        printf("文件中的内容如下：\n %s",context);
    }
    return;
}  

//从某位置开始释放磁盘空间
void freeDisk(int startPosition){
    //直接使用'\0'将char数组填充
    memset(disk,'\0',1024*Blocksize-startPosition);
    return;
}

//展示磁盘的使用情况
void diskShow(){
    //先展示distNode部分
    printf("前48个磁盘块存放distNode,均被使用\n");
    //查看第49块 used表示已有的用户
    printf("第49块磁盘上存放%d个MFD\n",used);
    //第50块中的UFD实际上是和MFD一样的
    printf("第50块磁盘上存放%d个UFD\n",used);
    //第51-65块中查看FCB的个数
    FCB* checking = (FCB*)FCB_Start;
    FCB* start = checking;
    //使用used变量查看文件个数
    while((checking-start)<10*used*sizeof(FCB)){
        if(checking->fileName[0] != '\0'){
            //已经完成查找的字节
            int finished = int(checking-start);
            checking = start + (finished/Blocksize)*Blocksize;
            printf("第%d块磁盘上存放有FCB\n",(finished/Blocksize+51));
        }
        else{
            checking+=sizeof(FCB);
        }
    }
    //遍历第66个磁盘块及其之后所有的字节
    char* element = (char*)Text_Start;
    char* ele_start = element;
    while((element-ele_start)<10*used*6*Blocksize){
        if(*element != '\0'){
            int finished = int(element - ele_start);
            element = ele_start+(finished/Blocksize)*Blocksize;
            printf("第%d块磁盘上存放有constext\n",(finished/Blocksize+66));
        }
        else{
            element+=sizeof(char);
        }
    }
}                    

int main(){

    time(&raw_time);

    initDisk();


    return 0;
}