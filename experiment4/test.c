#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

struct comb{
    int x;
    int y;
}comb;

void* thread_function(void* arg){
    int i;
    i=arg==NULL?0:(int)(*(int*)arg);
    for(;i<20;i++){
        printf("Thread%d says hi!\n",i);
        sleep(1);
    }
    return NULL;
}
void* thread_function0(void* arg){
    int i,j;
    if(arg==NULL){
        i=0;j=0;
    }
    else{
        struct comb co;
        co = (struct comb)(*(struct comb*)arg);
        i = co.x;
        j = co.y;
    }
    while(i+j<20){
        printf("The third thread%d,%d says hi!",i,j);
        i++;j++;
        sleep(1);
    }
}

int main(){
    pthread_t mythread1,mythread2,mythread3;
    void *ret1,*ret2,*ret3;
    int i=10;
    struct comb co;
    co.x=3;
    co.y=4;

    if(pthread_create(&mythread1,NULL, thread_function,(void*)&i)){
        printf("error creating thread.");
        abort();
    }    
    if(pthread_create(&mythread2,NULL, thread_function,NULL)){
        printf("error creating thread.");
        abort();
    }
    if(pthread_create(&mythread3,NULL, thread_function0,(void*)&co)){
        printf("error creating thread.");
        abort();
    }

    if(pthread_join(mythread1,NULL)){
        printf("error joining thread.");
        abort();
    }
    if(pthread_join(mythread2,NULL)){
        printf("error joining thread.");
        abort();
    }
    if(pthread_join(mythread3,NULL)){
        printf("error joining thread.");
        abort();
    }

    exit(0);
}