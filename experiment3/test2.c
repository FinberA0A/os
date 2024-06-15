#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>

int main(){
    pid_t pid = fork();
    char* args[] = {"ls","-l",NULL};

    if(pid>0){
        printf("this is a parent process\n");
    }
    else if(pid == 0){
        printf("i am child process, pid:%d, ppid:%d\n",getpid(),getppid());
        execvp("ls",args);
        perror("execvp");
        exit(42);
    }
    return 0;
}