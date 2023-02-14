#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 5

typedef int buffer_item;

void *producer(void *params);
void *consumer(void *params);

int insert_item(buffer_item item);
int remove_item(buffer_item *item);

void print_buffer();

//Define global variables
buffer_item buffer[BUFFER_SIZE];
sem_t full;
sem_t empty;
int in;
int out;
pthread_mutex_t mutex;

void *producer(void* params){
    buffer_item item;

    while(1){
        sleep(rand()%10);
        item = rand();

        if(insert_item(item)){
            printf("There was an error inserting the item in the buffer.\n");
            exit(1);
        }
        else{
            printf("The producer produced item: %d\n",(int) item);
            print_buffer();
        }
        return 0;

    }
}

void *consumer(void* params){
    buffer_item item;
    while(1){
        sleep(rand()%10);
        
        if(remove_item(&item)){
            printf("There was an error removing an item.\n");
            exit(1);
        }
        else{
            printf("The removed item was %d\n", item);
            print_buffer();
        }

        
    }
}

int insert_item(buffer_item item){
    sem_wait(&empty); //Wait for at least one empty space
    pthread_mutex_lock(&mutex); //Secure the CS of code. More than 1 producer

    buffer[in] = item;
    in = (in + 1) % BUFFER_SIZE; //Use mod to create a circular buffer

    //Exit the CS 
    pthread_mutex_unlock(&mutex);
    //Increase the number of buffer slots that need to be read.
    sem_post(&full);
    return 0;
}

int remove_item(buffer_item* item){
    sem_wait(&full); //Wait for at least one item to be place in the buffer
    pthread_mutex_lock(&mutex); //Secure the CS

    *item = buffer[out];
    buffer[out] = -1;
    out = (out + 1) % BUFFER_SIZE;

    //Release the CS
    pthread_mutex_unlock(&mutex);
    sem_post(&empty); //You emptied a buffer slot. Notify the producers/
    return 0;

}

void print_buffer(){
    printf("\t BUFFER:");
    for(int i = 0; i < BUFFER_SIZE; i++){
        printf(" %10d", buffer[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[]){
    if (argc != 4){
        printf("USAGE: [length of run in seconds] [num prods] [num cons]\n");
        exit(1); //End the program because of a USAGE error
    }

    //Initialize the variable
    in = 0;
    out = 0;
    sem_init(&full, 0, 0); //The buffer is empty, so full starts at 0
    sem_init(&empty, 0, BUFFER_SIZE); //The buffer is empty, so empty is the buffer size
    pthread_mutex_init(&mutex, NULL); //Mutex is initialized in the unlocked state.
    //Initialize buffer to -1 so we can tell if we are actually writing to it
    for (int i = 0; i<BUFFER_SIZE; i++){
        buffer[i] = -1;
    }


    int run_time = atoi(argv[1]);
    int num_prods = atoi(argv[2]);
    int num_cons = atoi(argv[3]);

    pthread_t prod_pids[num_prods];
    pthread_t cons_pids[num_cons];

    for (int i = 0; i<num_prods; i++){
        pthread_create(&prod_pids[i], NULL, producer, NULL);
    }

    for(int j = 0; j<num_cons; j++){
        pthread_create(&cons_pids[j], NULL, consumer, NULL);
    }

    sleep(run_time); //Sleep the main thread for the run time of the process

    return 0;
}