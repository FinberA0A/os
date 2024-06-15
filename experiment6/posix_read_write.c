#include<stdio.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<semaphore.h>
#include<pthread.h>

#define SHM_NAME "/shm"

// int main(){
//     int fd = shm_open("/shm", O_CREAT|O_RDWR, 0666);

//     ftruncate(fd, 4096);

//     char *ptr = mmap(0, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

//     sprintf(ptr, "hello\n");

//     printf("%s", ptr);

//     shm_unlink("/shm");
//     return 0;
// }

int readcount, writecount;
sem_t x,y,rsem,wsem;

void read_unit(int arg, int argm){
    
    int fd = shm_open(SHM_NAME, O_CREAT|O_RDWR, 0666);

    ftruncate(fd, 4096);

    char *ptr = mmap(0, 4096, PROT_READ, MAP_SHARED, fd, 0);

    // sprintf(ptr, "hello\n");

    printf("reader%d %d:%s\n", arg,argm,ptr);

    close(fd);

    //shm_unlink("SHM_NAME");

}

void write_unit(int arg){

    int fd = shm_open(SHM_NAME, O_CREAT|O_RDWR, 0666);

    ftruncate(fd, 4096);

    char *ptr = mmap(0, 4096, PROT_WRITE, MAP_SHARED, fd, 0);

    sprintf(ptr, "hello%d ",arg);

    // printf("%s", ptr);

    close(fd);

}

void* reader1(){
    int i=0;
    while(i<200){
        sem_wait(&rsem);
        sem_wait(&x);
        readcount++;
        if(readcount == 1) sem_wait(&wsem);
        sem_post(&x);
        sem_post(&rsem);
        
        read_unit(1,i);

        printf("reader1:%d, readcount:%d\n", i,readcount);
        sem_wait(&x);
        readcount--;
        if(readcount == 0) sem_post(&wsem);
        sem_post(&x);
        i++;
    }
}

void* reader2(){
    int i=0;
    while(i<200){
        sem_wait(&rsem);
        sem_wait(&x);
        readcount++;
        if(readcount == 1) sem_wait(&wsem);
        sem_post(&x);
        sem_post(&rsem);
        
        read_unit(2,i);

        printf("reader1:%d, readcount:%d\n", i,readcount);
        sem_wait(&x);
        readcount--;
        if(readcount == 0) sem_post(&wsem);
        sem_post(&x);
        i++;
    }
}

void* writer(void* args){
    int i=0;
    while(i<200){
        sem_wait(&y);
        writecount++;
        if(writecount==1) sem_wait(&rsem);
        sem_post(&y);

        sem_wait(&wsem);
        
        write_unit(i);
        printf("writer:%d, write once\n",i);
        i++;
        
        sem_post(&wsem);
        sem_wait(&y);
        writecount--;
        if(writecount == 0) sem_post(&rsem);
        sem_post(&y);

    }
}

int main(){
    readcount = 0;
    pthread_t thread[3];
    sem_init(&x,0,1);
    sem_init(&y,0,1);
    sem_init(&wsem,0,1);
    sem_init(&rsem,0,1);

    pthread_create(&thread[0],NULL,reader1,NULL);    
    pthread_create(&thread[1],NULL,writer,NULL);
    pthread_create(&thread[2],NULL,reader2,NULL);

    pthread_join(thread[0],NULL);
    pthread_join(thread[1],NULL);
    pthread_join(thread[2],NULL);
    
    sem_destroy(&x);
    sem_destroy(&wsem);
    shm_unlink("SHM_NAME");
    return 0;
}