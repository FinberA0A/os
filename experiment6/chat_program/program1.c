#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>

typedef struct{
    long mtype;
    char mtext[100];
}MyMsg;

void sendmsg(int msgid, MyMsg* msg_p){
    printf("p1:enter the context:");
    scanf("%s",msg_p->mtext);
    printf("text:%s\n",msg_p->mtext);
    msgsnd(msgid,msg_p,sizeof(MyMsg),0);
}

void receivemsg(int msgid, MyMsg* msg_p){
    printf("p1:receive a msg:");
    msgrcv(msgid,msg_p,sizeof(MyMsg),1,0);
    printf("%s\n",msg_p->mtext);
}

int main(){
    key_t key = ftok("/home/finber/usingfolder/os-folder/experiment6/chat_program",0);
    int msgid;
    int func;
    MyMsg msg;
    msg.mtype = 1;
    msgid = msgget(key,0666|IPC_CREAT);
    printf("msgid:%d\n", msgid);
    while(1){
        printf("p1:select the function: 1 for send; 2 for receive\n");
        scanf("%d",&func);
        if(func == 1){
            sendmsg(msgid, &msg);
        }
        else if(func == 2){
            receivemsg(msgid, &msg);
        }
        else{
            break;
        }
    }
    printf("program exit\n");
    return 0;
}