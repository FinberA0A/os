#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>

#define NUM_THREADS 5

sem_t semaphore;

void* thread_function(void* arg){
    int thread_id = *((int*)arg);
    printf("Thread %d is waiting for semaphore.\n",thread_id);
    sem_wait(&semaphore);
    printf("Thread %d acquired semaphore.\n",thread_id);
    sem_post(&semaphore);
    pthread_exit(NULL);
}

int main(){
    pthread_t threads[NUM_THREADS];
    sem_init(&semaphore, 0, 1);
    int thread_ids[NUM_THREADS];
    int i;
    for(i=0; i<NUM_THREADS; i++){
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, thread_function, &thread_ids[i]);
    }
    for(i=0; i<NUM_THREADS; i++){
        pthread_join(threads[i],NULL);
    }
    sem_destroy(&semaphore);
    return 0;
}