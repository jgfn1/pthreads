#define _XOPEN_SOURCE 600 //define para barreiras....
/*
		Thread Niggas
		Question 6 for Threads Project
		Created by nmf2
*/
#include <stdio.h>
#include <stdlib.h>
#include <sched.h> //sched_yield() comes from here, pthread_yield() wasn't working
#include <time.h>
#include <pthread.h>
#define N 3
#define M 100
#define S 2

typedef
struct Sensor{
    pthread_t thread;
    int blocked; //denotes if the sensor is blocked or not (i.e. to know if it's possible not to put any luggage in its queue)
    int count;
    int queue;
    pthread_mutex_t mutex; //mutex to add or remove luggages from queue
} Sensor;

Sensor sensor[S] = { {(pthread_t) NULL, 0, 0, 0, PTHREAD_MUTEX_INITIALIZER} };
pthread_mutex_t xrayedLuggageMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t blockedSensorsMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t doMaintenceCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t maintenceCompleteCond = PTHREAD_COND_INITIALIZER;
pthread_barrier_t maintenceCompleteBarrier, doMaintenceBarrier;
int xrayedLuggageCount = 0;
int blockedSensors = 0;

pthread_t line[N];
pthread_mutex_t lineMutex[N] = PTHREAD_MUTEX_INITIALIZER;
pthread_t control;

int getLeastBusySensor(int caller){
    int i = 0, result = 0;
    //printf("GLBS Caller: %d\n", caller);
    if(S == 1) return 0;

    for(i = 0; i < S; i++){ //lock all sensors
        pthread_mutex_lock(&sensor[i].mutex);
        printf("GLSB: Caller %d acquired sensor %d: \n\tcount - %d, queue - %d, blocked - %d\n", caller, i, sensor[i].count, sensor[i].queue, sensor[i].blocked);
    }

    for (i = 0; i < S-1; i++){
        if(sensor[i].blocked == 0){ //if the first sensor is not blocked
            if(sensor[i+1].blocked == 0){ //and the second sensor is not blocked
                if(sensor[i].queue < sensor[i+1].queue)//compare the size of their queues and select the queue with the lower size
                    result = i;
                else
                    result = i + 1;
            } else {
                result = i; //the least busy sensor will be the one not blocked
            }
        } else {
            result = i + 1;
        }
    }

    for(i = S-1; i >= 0; i--){ //lock all sensors
        if (i != result){
            pthread_mutex_unlock(&sensor[i].mutex);
            //printf("GLSB: Caller %d realeased sensor %d \n", caller, i);
        }
    }
    return result;
}

void * xrayLuggage(void * s){
    int this = (int) s; //this sensor's index
    printf("sensor %d started\n", this);
    while (xrayedLuggageCount < N*M){
        pthread_mutex_lock(&sensor[this].mutex);
        if (sensor[this].blocked == 0){
            if(sensor[this].count < 20){ //if there's no need for maintence
                if(sensor[this].queue > 0){
                    sensor[this].queue--;
                    sensor[this].count++;
                    pthread_mutex_lock(&xrayedLuggageMutex);
                    xrayedLuggageCount++;
                    pthread_mutex_unlock(&xrayedLuggageMutex);
                    printf("sensor: %d, count: %d, queue: %d, total count: %d\n", this, sensor[this].count, sensor[this].queue, xrayedLuggageCount);
                }
            } else{ //block sensor and start or wait for maintence
                sensor[this].blocked = 1;
                pthread_mutex_lock(&blockedSensorsMutex);
                blockedSensors++;
                pthread_mutex_unlock(&blockedSensorsMutex);
                while (sensor[this].blocked) {
                    printf("sensor %d waiting for maintence, state: %d. blockedSensors: %d\n", this, sensor[this].blocked, blockedSensors);
                    pthread_cond_wait(&doMaintenceCond, &sensor[this].mutex);
                    printf("\t\tSensor %d: Manutencao\n", this);
                    sensor[this].count = 0;
                    sensor[this].blocked = 0;
					pthread_mutex_lock(&blockedSensorsMutex);
					blockedSensors--; //decrement count of blocked sensors
					pthread_mutex_unlock(&blockedSensorsMutex);
                }
                printf("sensor %d waiting for maintence to finish, blockedSensors: %d\n", this, blockedSensors);
                pthread_barrier_wait(&maintenceCompleteBarrier);
            }
        }
        pthread_mutex_unlock(&sensor[this].mutex);
        //sched_yield();
    }
}

void * controlThread(){
    while(xrayedLuggageCount < N*M){
        int i;
        if(blockedSensors == S){
            for(i = 0; i < N; i++){ //lock all lines to make sure they're not trying to lock the sensors
                pthread_mutex_lock(&lineMutex[i]);
                printf("Control acquired line %d \n", i);
            }
            for(i = 0; i < S; i++){ //lock all sensors to make sure they are waiting for doMaintenceCond
                pthread_mutex_lock(&sensor[i].mutex);
                printf("Control acquired sensor %d: \n\tcount - %d, queue - %d, blocked - %d\n", i, sensor[i].count, sensor[i].queue, sensor[i].blocked);
            }

            pthread_cond_broadcast(&doMaintenceCond); //allow maintence to happen

            for(i = S-1; i >= 0; i--){ //unblock all sensors for the maintence to actually happen
                pthread_mutex_unlock(&sensor[i].mutex);
                printf("Control realeased sensor %d \n", i);
            }

            pthread_barrier_wait(&maintenceCompleteBarrier); //wait for the maintence to finish

            printf("\tAviso: Todos os sensores receberam manutencao.\n"); //print message when its done

            for(i = N-1; i >= 0; i--){ //unblock all lines and get everything back to work
                pthread_mutex_unlock(&lineMutex[i]);
                printf("Control realeased line %d \n", i);
            }
        }
    }
}

void * sendLuggageToSensor(void * line){
    int this = (int) line;
    printf("line %d started\n", this);
    int remainingLuggage = M;
    int s = 0;
    while (remainingLuggage > 0) {
		while (blockedSensors == S) { //it's of no use to run this now, all sensors are blocked
			sched_yield();
		}
        pthread_mutex_lock(&lineMutex[this]);
        s = getLeastBusySensor(this); //mutex still locked
        //no need to lock sensor[s] because it's still locked from the lsb
        printf("line %d, sensor: %d, lock acquired\n", this, s);
        if(sensor[s].queue < 5){
            sensor[s].queue++;
            remainingLuggage--;
            //printf("Line %d: sensor[%d].queue = %d, remaining: %d\n", this, s, sensor[s].queue, remainingLuggage);
        }
        pthread_mutex_unlock(&sensor[s].mutex); //unlock sensor that's still locked from getLeastBusySensor()
        printf("line %d, sensor: %d, lock realeased\n", this, s);
        pthread_mutex_unlock(&lineMutex[this]);
        sched_yield();
    }
}

int main(int argc, char const *argv[]) {
    int i;
    pthread_barrier_init(&maintenceCompleteBarrier, NULL, S+1); //all sensors plus control thread
    pthread_barrier_init(&doMaintenceBarrier, NULL, S);
    pthread_create(&control, NULL, controlThread, NULL);
    for(i = 0; i < N; i++){
        pthread_create(&line[i], NULL, sendLuggageToSensor, (void *) i);
    }
    for(i = 0; i < S; i++){
        pthread_create(&sensor[i].thread, NULL, xrayLuggage, (void*) i);
    }
    pthread_exit(NULL);
}
