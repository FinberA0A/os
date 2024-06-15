#include<stdio.h>
#include<stdlib.h>
//#include<pthread.h>
#include<time.h>
#include<unistd.h>

int main(){
    clock_t start,end;
    int n;
    long long sum=0;
    printf("pls input the n:");
    scanf("%d",&n);
    start = clock();
    for(int i=0; i<=n; i++){
        sum+=i;
    }
    end = clock();
    printf("result:%lld\n",sum);
    printf("time=%f\n",(double)(end-start));
    return 0;
}