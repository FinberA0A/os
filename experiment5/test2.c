#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>

sem_t semaphore;
sem_t buffer;

void* produce(void* args){
    for(int i=1; i<=20; i++){
        printf("producer waiting\n");
        sem_wait(&semaphore);
        sem_post(&buffer);
        printf("producer %d times acquire\n",i);
        sem_post(&semaphore);      
    }

}

void* consume(void* args){
    for(int i=1; i<=20; i++){
        printf("consumer waiting\n");
        sem_wait(&semaphore);
        sem_wait(&buffer);
        printf("consumer %d times acquire\n",i);
        sem_post(&semaphore);           
    }
}

int main(){
    pthread_t thread[2];
    sem_init(&semaphore, 0, 1);
    sem_init(&buffer, 0, 0);

    pthread_create(&thread[0],NULL,produce,NULL);
    pthread_create(&thread[1],NULL,consume,NULL);

    pthread_join(thread[0],NULL);
    pthread_join(thread[1],NULL);
    sem_destroy(&semaphore);
    sem_destroy(&buffer);
    return 0;
}