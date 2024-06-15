#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>

int main(){

    pid_t pid=fork();
    int num=0;
    int status;
    
    if(pid>0){
        printf("i am parent process, pid:%d, ppid:%d\n",getpid(),getppid());
        printf("parent num:%d\n",num);
        num+=10;
        printf("parent num+=10:%d\n",num);
        printf("fork return value:%d\n",pid);

        waitpid(pid,&status,0);

        if(WIFEXITED(status)){
            printf("child process exit:%d\n",WEXITSTATUS(status));
        }

        printf("parent sleep 60s\n");
        sleep(60);

        exit(0);
    }
    else if(pid == 0){
        printf("i am child process, pid:%d, ppid:%d\n",getpid(),getppid());
        printf("child num:%d\n",num);
        num+=100;
        printf("child num+=100:%d\n",num);
        printf("fork return value:%d\n",pid);

        printf("child sleep 60s\n");
        sleep(60);

        exit(42);
    }
    //else{

    //}
    return 0;
}