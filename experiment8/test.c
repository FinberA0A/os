#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#define TRUE 1
#define FALSE 0
#define INVALID -1
#define total_instruction 320 //模拟的指令书
#define total_vp 32           //模拟的虚拟页面数

typedef struct{  //页面结构
    int pn;      //页号
    int pfn;     //内存块号
    int counter; //一个周期内访问页面的次数
    int time;    //访问时间     
}pl_type;
pl_type pl[total_vp];

typedef struct pfc_struct{  //页面控制结构
    int pn;                 //页号
    int pfn;                //内存块号
    int time;               //访问时间
    int next_range;         //下一次访问的距离
    struct pfc_struct* next;
}pfc_type;
pfc_type pfc[total_vp];     //用户进程虚页控制结构
pfc_type *freepf_head;      //空内存页头指针
pfc_type *busypf_head;      //忙内存页头指针
pfc_type *busypf_tail;      //忙内存页尾指针
int pfc_num;                //pfc中已经占用的数目

int disaffect;              //页面失效次数
int a[total_instruction];   //指令流数据组
int page[total_instruction];//每条指令所属页号
int offset[total_instruction];//每页装入10条指令后取得的页号偏移量

void initialize();          //初始化数据
void FIFO();                //计算使用FIFO的访问命中率
void LRU();                 //计算使用LRU访问命中率
void OPT();                 //计算使用OPT访问命中率
void time_increase();       //增加pfc中的访问时间
int LRU_space();            //在已满的pfc空间中筛选出时间最久的并进行空间变化
void caculate_ranges();     //计算pfc中下一次调用的间隔
int OPT_space();            //在已满的pfc空间中筛选出下一次调用时间间隔最长的


int main(){
    int s,i,j;
    srand(10*getpid());
    s = (float)319*rand()/32767/32767/2 + 1;    //将指令限制在1～320之间
    //printf("rand_max:%d\n",RAND_MAX/32767/32767/2);
    for(i = 0; i<total_instruction; i+=4){//通过随机函数生成320条指令
       if(s<0||s>319){
           printf("when i==%d, Error, s==%d\n", i, s);
           exit(0);
        }
        a[i] = s;
        a[i+1] = a[i]+1;
        a[i+2] = (float)a[i]*rand()/32767/32767/2;
        a[i+3] = a[i+2]+1;
        s = (float)(318-a[i+2])*rand()/32767/32767/2 + a[i+2] + 2;
        if((a[i+1]>318)||(s>319)){
            printf("a[%d+2], a number which is: %d and s==%d\n",i, a[i+2],s);
        }        
    }
    //将指令序列转换为页面地址流（每一页存放10条指令，使用page和offset精准定位）
    for(i=0; i<total_instruction; i++){
        page[i] = a[i]/10;
        offset[i] = a[i]%10;
    }
    //用户工作区从4个页面变换到32个页面
    for(i=4; i<=32; i++){
        printf("%2d page frames ",i);
        FIFO(i);   //按照先进先出的方法
        printf("\n");
    }
    printf("\n");
    for(i=4; i<=32; i++){
        printf("%2d page frames ",i);
        LRU(i);    //评估pfc中最长时间未被使用
        printf("\n");
    }
    printf("\n");
    for(i=4; i<=32; i++){
        printf("%2d page frames ",i);
        OPT(i);    //根据pfc中的替换未来调用最远的
        printf("\n");
    }
}

void initialize(int total_pf){
    int i;
    disaffect = 0;
    for(i=0; i<total_vp; i++){
        pl[i].pn = i;
        pl[i].pfn = INVALID;
        pl[i].counter = 0;
        pl[i].time = -1;
    }
    for(i=0; i<total_pf-1; i++){
        pfc[i].next = &pfc[i+1];
        pfc[i].pfn = i;
    }
    pfc[total_pf-1].next=NULL;
    pfc[total_pf-1].pfn = total_pf-1;
    freepf_head = &pfc[0];
}

void FIFO(int total_pf){
    int i,j;
    pfc_type *p;
    initialize(total_pf);
    busypf_head = busypf_tail = NULL;
    for(i=0; i<total_instruction; i++){
        if(pl[page[i]].pfn==INVALID){ //页面失效
            disaffect+=1;
            if(freepf_head == NULL){  //无空闲页面
                p=busypf_head->next;
                pl[busypf_head->pn].pfn = INVALID;
                freepf_head = busypf_head;   //释放忙页面的第一个页面
                freepf_head->next = NULL;
                busypf_head = p;
            }
            p = freepf_head->next;           //按FIFO方式将新页面调入内存页面
            freepf_head->next = NULL;
            freepf_head->pn = page[i];
            pl[page[i]].pfn = freepf_head->pfn;
            if(busypf_tail == NULL)
                busypf_head = busypf_tail = freepf_head;
            else{
                busypf_tail->next = freepf_head;    //减少一个空闲页面
                busypf_tail = freepf_head;
            }
            freepf_head=p;
        }
    }
    printf("FIFO:%6.4f",1-(float)disaffect/320);
}

void LRU(int total_pf){
    //total_pf是指一共的页号
    int i,to_replace;
    initialize(total_pf);   //初始化pl和pfc
    pfc_num = 0;
    for(i=0; i<total_instruction; i++){
        time_increase(pfc_num);    //增加pfc中的时间
        if(pl[page[i]].pfn == INVALID){  //页面失效
            disaffect+=1;
            to_replace = pfc_num;
            if(pfc_num == total_pf){     //pfc区域已经满了
                to_replace = LRU_space();             //将已经在pfc中的最长时间没有访问的置换
                pfc_num--;
                pl[pfc[to_replace].pn].pfn = INVALID; //将pl中的被to_replace的页进行INVALID
            }
            //将新的访问的放入pfc
            pfc_num++;
            pfc[to_replace].pn = page[i];
            pfc[to_replace].time = 0;
            pl[page[i]].pfn = to_replace;
        }
        else{                      //页面存在，重置其时间
            pfc[pl[page[i]].pfn].time = 0;
        }
    }
    printf("LRU:%6.4f",1-(float)disaffect/320);
}

void time_increase(int pfc_num){
    //增加前pfc_num个pfc中的time
    int i;
    for(i=0; i<pfc_num; i++){
        pfc[i].time+=1;
    }
}

int LRU_space(){
    //遍历pfc_num个pfc结构
    int i, longest;
    int time = -1;
    longest = 0;
    for(i=0; i<pfc_num; i++){
        if(time<pfc[i].time){
            time = pfc[i].time;
            longest = i;
        }
    }
    return longest;
}

void OPT(int total_pf){
    //total_pf是指一共的页号
    int i,to_replace;
    initialize(total_pf);   //初始化pl和pfc
    pfc_num = 0;
    for(i=0; i<total_instruction; i++){
        caculate_ranges(pfc_num, i);    //计算下一次调用该pfc时的跨度
        if(pl[page[i]].pfn == INVALID){  //页面失效
            disaffect+=1;
            to_replace = pfc_num;
            if(pfc_num == total_pf){     //pfc区域已经满了
                to_replace = OPT_space();             //将已经在pfc中的最长时间没有访问的置换
                pfc_num--;
                pl[pfc[to_replace].pn].pfn = INVALID; //将pl中的被to_replace的页进行INVALID
            }
            //将新的访问的放入pfc
            pfc_num++;
            pfc[to_replace].pn = page[i];
            pfc[to_replace].time = 0;
            pl[page[i]].pfn = to_replace;
        }
        //页面存在，不进行任何操作
    }
    printf("OPT:%6.4f",1-(float)disaffect/320);
}

void caculate_ranges(int pfc_num, int instr_num){
    int i,j;
    for(i=0; i<pfc_num; i++){
        int flag=1;
        for(j=instr_num; j<total_instruction; j++){
            if(pfc[i].pn == page[j]){
                pfc[i].next_range = j-instr_num;
                flag=0;
            }
            if(flag){     //若之后都不调用该pfc
                pfc[i].next_range = total_instruction-instr_num;
            }
        }
    }
} 

int OPT_space(){
    //遍历pfc_num个pfc结构
    int i, longest;
    int range = -1;
    longest = 0;
    for(i=0; i<pfc_num; i++){
        if(range<pfc[i].next_range){
            range = pfc[i].next_range;
            longest = i;
        }
    }
    return longest;
}