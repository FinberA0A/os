#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<time.h>
#include<unistd.h>

//making 2 thread
long long sum=0;
typedef struct {
    int x;
    int y;
    long long spart;
}comb;
long long s1;
long long s2;

void* thread_function(void* arg){

    int i,j;
    long long s=0;
    comb* co = ((comb*)arg);
    i = co->x;
    j = co->y;
    while(i<=j){
        s+=i;
        i++;
    }
    co->spart=s;
}

int main(){
    pthread_t mythread1;
    pthread_t mythread2;
    clock_t start,end;
    int n;
    comb co1,co2;
    printf("pls input the n:");
    scanf("%d",&n);
    co1.x=1;
    co1.y=n/2;
    co1.spart = 0;
    co2.x=n/2+1;
    co2.y=n;
    co2.spart = 0;

    start = clock();
    if(pthread_create(&mythread1,NULL,thread_function,(void*)&co1)){
        printf("creating 1st fail\n");
    }
    if(pthread_create(&mythread2,NULL,thread_function,(void*)&co2)){
        printf("creating 2nd fail\n");
    }
    
    pthread_join(mythread1,NULL);
    pthread_join(mythread2,NULL);
    sum = co1.spart+co2.spart;
    end = clock();
    
    printf("result:%lld\n",sum);
    printf("time=%f\n",(double)(end-start));
    return 0;
}