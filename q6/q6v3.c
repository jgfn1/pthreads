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
#define N 5
#define M 60
#define S 4

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
pthread_mutex_t reallocationArrayMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reallocationMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t doMaintenceCond[S] = { PTHREAD_COND_INITIALIZER };
pthread_cond_t maintenceCompleteCond = PTHREAD_COND_INITIALIZER;
pthread_barrier_t maintenceCompleteBarrier;

int xrayedLuggageCount = 0;
int blockedSensors = 0;
int sensorsPendingReallocation[S] = { 0 }; //0 means it doesn't need reallocation, 1 means it does
int sensorToReallocate = -1; //holds the index of the sensor that's having it's luggage reallocated in the present moment.



pthread_t line[N];
pthread_mutex_t lineMutex[N] = PTHREAD_MUTEX_INITIALIZER;
pthread_t control;

void lockAllLines(char * owner, int except){ //lock all lines but the one indicated by "except"
    int i;
    for(i = 0; i < N; i++){
        if(i != except){
            pthread_mutex_lock(&lineMutex[i]);
            printf("%s acquired line %d \n", owner, i);
        }
    }
}

void unlockAllLines(char * owner, int except){// unlock all lines but the one indicated by "except"
    int i;
    for(i = N-1; i >= 0; i--){
        if(i != except){
            pthread_mutex_unlock(&lineMutex[i]);
            printf("%s realeased line %d \n", owner, i);
        }
    }
}

void lockAllSensors(char * owner){ //lock all sensors but the one indicated by "except"
    int i;
    for(i = 0; i < S; i++){
        pthread_mutex_lock(&sensor[i].mutex);
        printf("%s acquired sensor %d: \n\tcount - %d, queue - %d, blocked - %d\n", owner, i, sensor[i].count, sensor[i].queue, sensor[i].blocked);
    }
}

void unlockAllSensors(char * owner){ //unlock all sensors but the one indicated by "except"
    int i;
    for(i = S-1; i >= 0; i--){
        pthread_mutex_unlock(&sensor[i].mutex);
        printf("%s realeased sensor %d \n", owner, i);
    }
}

void wait(int millisec){
    struct timespec ts;
    ts.tv_sec = millisec/1000; //quantity of seconds
    ts.tv_nsec = (millisec % 1000) * 1000000; // quantity of nanoseconds
    nanosleep(&ts, NULL); //wait for tv_sec seconds and tv_nsec nanoseconds
}

int getLeastBusySensor(int caller){
    int i = 0, result = 0;
    char owner[20] = {'\0'};
    printf("GLBS Caller: %d\n", caller);
    sprintf(owner, "GLBS Caller: %d", caller);

    if(S == 1) return 0;

    lockAllSensors(owner); // lock all sensors, no exceptions

    for (i = 0; i < S-1; i++){
        if(sensor[result].blocked == 0){ //if the first sensor is not blocked
            if(sensor[i+1].blocked == 0) //and the second sensor is not blocked
                if(sensor[i+1].queue < sensor[result].queue)//compare the size of their queues and select the queue with the lower size
                    result = i+1;
        } else{
            result = i+1;
        }
    }

    unlockAllSensors(owner); // unlock all sensors, no exceptions
    return result;
}

void * xrayLuggage(void * s){
    int this = (int) s; //this sensor's index
    printf("sensor %d started\n", this);
    while (xrayedLuggageCount < N*M){
        //printf("sensor %d try to accquire lock\n", this);
        pthread_mutex_lock(&sensor[this].mutex);
        printf("sensor %d acquired its lock\n", this);
        //printf("sensor: %d, count: %d, queue: %d, blocked: %d, total count: %d\n", this, sensor[this].count, sensor[this].queue, sensor[this].blocked, xrayedLuggageCount);
        if(sensor[this].count < 10){ //if there's no need for maintence
            if(sensor[this].queue > 0){
                wait(50);//wait for check
                sensor[this].queue--;
                sensor[this].count++;
                pthread_mutex_lock(&xrayedLuggageMutex);
                xrayedLuggageCount++;
                pthread_mutex_unlock(&xrayedLuggageMutex);
                //printf("sensor: %d, count: %d, queue: %d, total count: %d\n", this, sensor[this].count, sensor[this].queue, xrayedLuggageCount);
            }
        } else{ //block sensor and start or wait for maintence
            sensor[this].blocked = 1;

/*            if(pthread_mutex_trylock(&reallocationMutex) == 0) { //if the lock can be acquired that means that no line or other mutex is trying to write on it
                sensorToReallocate = this;
                pthread_mutex_unlock(&reallocationMutex);
            } else { //else, that variable is already being used, so just store info in the buffer.
                pthread_mutex_lock(&reallocationArrayMutex);
                sensorsPendingReallocation[this] = 1; //
                pthread_mutex_unlock(&reallocationArrayMutex);
            }*/

            pthread_mutex_lock(&blockedSensorsMutex);
            blockedSensors++;
            pthread_mutex_unlock(&blockedSensorsMutex);

            while (sensor[this].blocked) {
                printf("sensor %d waiting for maintence, state: %d. blockedSensors: %d, xraycount: %d\n", this, sensor[this].blocked, blockedSensors, xrayedLuggageCount);
                pthread_cond_wait(&doMaintenceCond[this], &sensor[this].mutex);

                printf("\t\tSensor %d: Manutencao\n", this);
                sensor[this].count = 0;
                sensor[this].blocked = 0;
/*
                pthread_mutex_lock(&reallocationArrayMutex);
                sensorsPendingReallocation[this] = 0; //since maintenance has just happened any pending reallocaton is unnecessary.
                pthread_mutex_unlock(&reallocationArrayMutex);
*/
                pthread_mutex_lock(&blockedSensorsMutex);
                blockedSensors--; //decrement count of blocked sensors
                pthread_mutex_unlock(&blockedSensorsMutex);
            }
            printf("sensor %d waiting for maintence to finish, blockedSensors: %d\n", this, blockedSensors);
            if(xrayedLuggageCount < N*M) pthread_barrier_wait(&maintenceCompleteBarrier);
        }
        pthread_mutex_unlock(&sensor[this].mutex);
        //printf("sensor %d Realeased its lock\n");
        //sched_yield();
    }
}


void * controlThread(){
    int i;
    while(xrayedLuggageCount < N*M){
        if(blockedSensors == S){

            lockAllLines("Control", -1); //the -1 arg makes sure that all lines will be locked, no exceptions

            lockAllSensors("Control"); //the -1 arg makes sure that all sensors will be locked, no exceptions

            for(i = 0; i < S; i++){
                pthread_cond_signal(&doMaintenceCond[i]);
            }

            unlockAllSensors("Control"); //the -1 arg makes sure that all sensors will be locked, no exceptions

            printf("Control waiting for maintence to finish\n");
            pthread_barrier_wait(&maintenceCompleteBarrier); //wait for the maintence to finish

            printf("\tAviso: Todos os sensores receberam manutencao.\n"); //print message when its done

            unlockAllLines("Control", -1); //the -1 arg makes sure that all lines will be locked, no exceptions
        }
    }

    printf("All done, control finishing.");
    for(i = 0; i < S; i++){ //prevents deadlocks while reallocation isn't being implemented
        pthread_cond_signal(&doMaintenceCond[i]);
    }
}
int getSensorPendingReallocation(){
    int i, result = -1; // the default is not having any sensors pending reallocation
    for (i = 0; i < S; i++){
        if(sensorsPendingReallocation[i] == 1){ //in case the sensor is pending reallocation
            result = i; //save it
            break; //stop the loop
        }
    }
    return result;
}

void reallocateLuggage(int line){
    char owner[20] = { '\0' };
    int leastBusy = 0;

    sprintf(owner, "Line %d", line); //puts string into variable

    lockAllLines(owner, line); //all other lines are blocked, because this operation has priority over any luggage coming from the them.

    leastBusy = getLeastBusySensor(line); //this line called the function and this line can't be locked

    if(leastBusy < sensorToReallocate){ //this prevents deadlocks with calls to lockAllSensors(), mutexes are always unlocked in increasing numerical order
        pthread_mutex_lock(&sensor[leastBusy].mutex);
        pthread_mutex_lock(&sensor[sensorToReallocate].mutex);
    } else {
        pthread_mutex_lock(&sensor[sensorToReallocate].mutex);
        pthread_mutex_lock(&sensor[leastBusy].mutex);
    }

    if(sensor[sensorToReallocate].queue > 0){
        if(sensor[leastBusy].queue < 5){
            sensor[sensorToReallocate].queue--; //remove luggage from the queue of the sensor which is pending reallocation
            sensor[leastBusy].queue++;          // put luggage into the queue of the least busy sensor
        }
    } else {//if there's no more luggage to reallocate for this sensor
        pthread_mutex_lock(&reallocationArrayMutex);

        if(leastBusy < sensorToReallocate){ //this prevents deadlocks with calls to lockAllSensors(), mutexes are always unlocked in increasing numerical order
            pthread_mutex_unlock(&sensor[sensorToReallocate].mutex);
            pthread_mutex_unlock(&sensor[leastBusy].mutex);
        } else {
            pthread_mutex_unlock(&sensor[leastBusy].mutex);
            pthread_mutex_unlock(&sensor[sensorToReallocate].mutex);
        }
        sensorToReallocate = getSensorPendingReallocation();

        pthread_mutex_unlock(&reallocationArrayMutex);

    }

    if(leastBusy < sensorToReallocate){ //this prevents deadlocks with calls to lockAllSensors(), mutexes are always unlocked in increasing numerical order
        pthread_mutex_unlock(&sensor[sensorToReallocate].mutex);
        pthread_mutex_unlock(&sensor[leastBusy].mutex);
    } else {
        pthread_mutex_unlock(&sensor[leastBusy].mutex);
        pthread_mutex_unlock(&sensor[sensorToReallocate].mutex);
    }

    unlockAllLines(owner, line);

}

void * sendLuggageToSensor(void * line){
    int this = (int) line;
    printf("line %d started\n", this);
    int remainingLuggage = M;
    int s = 0;
    while (xrayedLuggageCount != N*M) { //lines must finish only when all sensors finish because they might have to reallocate luggage
        // while (blockedSensors == S) { //it's of no use to run this now, all sensors are blocked
        //  sched_yield();
        // }
        
        /*pthread_mutex_lock(&reallocationMutex);
        if(sensorToReallocate != -1){//if there's a sensor pending reallocation
            printf("Sensor %d pending reallocation: \n\tcount - %d, queue - %d, blocked - %d\n", sensorToReallocate, sensor[sensorToReallocate].count, sensor[sensorToReallocate].queue, sensor[sensorToReallocate].blocked);
            reallocateLuggage(this);
        }
        pthread_mutex_unlock(&reallocationMutex);
*/
        pthread_mutex_lock(&lineMutex[this]);
        s = getLeastBusySensor(this);
        pthread_mutex_lock(&sensor[s].mutex);
        //the -1 makes sure no exceptions are made, that is, all sensors are locked and considered in the determination of which is the least busy
        printf("line %d, sensor %d: queue: %d, blocked: %d lock acquired, the rest:\n", this, s, sensor[s].queue, sensor[s].blocked);
        //for(i=0;i<S;i++)printf("line %d, sensor %d: queue: %d, blocked: %d lock acquired\n", this, i, sensor[i].queue, sensor[i].blocked);

        if(sensor[s].queue < 5){
            sensor[s].queue++;
            remainingLuggage--;//remove luggage from this line's remaining queue
            printf("Line %d: sensor[%d].queue = %d, remaining: %d\n", this, s, sensor[s].queue, remainingLuggage);
        }
        pthread_mutex_unlock(&sensor[s].mutex); //unlock sensor that's still locked from getLeastBusySensor()
        printf("line %d, sensor: %d, lock realeased\n", this, s);
        pthread_mutex_unlock(&lineMutex[this]);
        //sched_yield();
    }
    printf("line %d finished, xrayedLuggageCount: %d\n", this, xrayedLuggageCount);
    //wait(100000);
}

int main(int argc, char const *argv[]) {
    int i;
    pthread_barrier_init(&maintenceCompleteBarrier, NULL, S+1); //all sensors plus control thread
    pthread_create(&control, NULL, controlThread, NULL);
    for(i = 0; i < N; i++){
        pthread_create(&line[i], NULL, sendLuggageToSensor, (void *) i);
    }
    for(i = 0; i < S; i++){
        pthread_create(&sensor[i].thread, NULL, xrayLuggage, (void*) i);
    }
    pthread_exit(NULL);
}
