#define _XOPEN_SOURCE 600 //define para barreiras....
/*
		Thread Niggas
		Question 6 for Threads Project
		Created by nmf2
*/
#include <stdio.h>
#include <stdlib.h>
#include <sched.h> //sched_yield() comes from here
#include <time.h>
#include <pthread.h>
#define N 3
#define M 10
#define S 2

void printMessageOnce(char * message);
void * xrayLuggage(void * id);
int getLeastBusySensor(int line);
void * sendLuggageToSensor(void * id);
void reallocateLuggage(int s);
void wait(int milisec);

typedef
struct Sensor{
    pthread_t thread;
    int blocked; //denotes if the sensor is blocked or not (i.e. to know if it's possible not to put any luggage in its queue)
    int count; //counts how many luggages where checked. (for maintence...)
    pthread_mutex_t mutex; //mutex to add or remove luggages from queue
} Sensor;

pthread_t lines[N], control;
Sensor sensor[S] = {{ (pthread_t) NULL, 0, 0, PTHREAD_MUTEX_INITIALIZER}};
pthread_mutex_t reallocationMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sensorsMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t maintenceMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t xrayedLuggageMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t maintence, maintenceCompleteBarrier, maintenceMessage, startup;
pthread_cond_t doMaintenceCond = PTHREAD_COND_INITIALIZER;

int sensorToReallocateLuggage = -1; //holds sensor that needs to have its luggage reallocated
int sensorsQueueOccupied[S] = { 0 };  //each sensor has a queue, this is how many places of each queue is occupied
int xrayedLuggage = 0;
int dispatchedLuggage = 0;
int maintenceCount = 0;


/***
*		@getLeastBusySensor
*		@description: Analyzes sensorsQueueOccupied to determine which sensor queue is the least
*		busy and returns its index;
***/

int getLeastBusySensor(int line ){
    int i = 0, leastBusySensor = -1;

    if(S == 1){
        //printf("#############Realeased by getLeastBusySensor\n");
        pthread_mutex_lock(&sensor[0].mutex);
        printf("getLeastBusySensor Acquired the mutex of sensor 0\n");
        if (sensor[0].blocked){
            leastBusySensor = -1;
        } else {
            leastBusySensor = 0;
        }
        pthread_mutex_unlock(&sensor[0].mutex);
        return leastBusySensor;
    }
    //printf("#############owned by getLeastBusySensor\n");
    for(i = 0; i < S; i++){ //lock all sensors
        pthread_mutex_lock(&sensor[i].mutex);
        printf("Acquired the mutex of sensor %d, line %d\n", i, line);
    }
    pthread_mutex_lock(&sensorsMutex);
    printf("sensorsMutex accquired by getLeastBusySensor, line %d\n", line);

    for(i = 0; i < S - 1; i++){
        if(sensor[i].blocked == 0){ //if the first sensor is not blocked
            if(sensor[i+1].blocked == 0){ //and the second sensor is not blocked
                if(sensorsQueueOccupied[i] < sensorsQueueOccupied[i+1])//compare the size of their queues and select the queue with the lower size
                    leastBusySensor = i;
                else
                    leastBusySensor = i + 1;
            } else {
                leastBusySensor = i; //the least busy sensor will be the one not blocked
            }
        }//if not, just go on to the next test (iteration)
    }
    pthread_mutex_unlock(&sensorsMutex);
    printf("sensorsMutex Realeased by getLeastBusySensor, line %d\n", line);
    for(i = S-1; i >= 0; i--){//unlock all sensors
        pthread_mutex_unlock(&sensor[i].mutex);
        printf("Realeased the mutex of sensor %d, line %d\n", i, line);
    }

    //printf("#############Realeased by getLeastBusySensor\n");

    return leastBusySensor; //if all sensors are blocked, -1 will be returned
}
int getLeastBusySensorB(int line){
    int i = 0, leastBusySensor = 0;
    if (S == 1) return 0; //if there's only one available sensor, it is the least busy one.
    pthread_mutex_lock(&sensorsMutex);
    for (i = 0; i < S-1; i++){
        if(sensorsQueueOccupied[i] < sensorsQueueOccupied[i + 1]) //compare sensors to know which has the lower queue size
            leastBusySensor = i;
        else
            leastBusySensor = i + 1;
    }
    pthread_mutex_unlock(&sensorsMutex);
    return leastBusySensor;
}

/***
*		@sendLuggageToSensor
*		@description: Sends luggage to the least busy sensor queue;
***/
void * sendLuggageToSensor(void * id){
    pthread_barrier_wait(&startup);
    int l = (int) id; //get line index
    int remaining = M;
    while(remaining > 0){
        int s = getLeastBusySensor(l); //gets the least busy and unblocked sensor
        if(s != -1){
            pthread_mutex_lock(&sensorsMutex);
            printf("sensorsMutex locked by line %d thread\n", s);
            if(sensorsQueueOccupied[s] < 5){ //if the least busy queue has less than 5 elements in it
                pthread_mutex_lock(&reallocationMutex);
                if(sensorToReallocateLuggage == -1) { //if no sensor needs luggage reallocating
                    remaining--;//decrement line's remaining luggage count
                } else { //reallocate
                    if(sensorsQueueOccupied[sensorToReallocateLuggage] > 0){
                        printf("sensor to reallocate luggage: %d, quantity: %d\n", sensorToReallocateLuggage, sensorsQueueOccupied[sensorToReallocateLuggage]);
                        sensorsQueueOccupied[s] += 1; //increment sensor queue size
                        sensorsQueueOccupied[sensorToReallocateLuggage] -= 1;
                    } else {
                        printf("reallocating sensor %d done\n",sensorToReallocateLuggage);
                        sensorToReallocateLuggage = -1;
                    }
                }
                sensorsQueueOccupied[s] += 1; //increment sensor queue size
                pthread_mutex_unlock(&reallocationMutex);
            }//else, it means that all sensors are full	y occupied: nothing to do.
            pthread_mutex_unlock(&sensorsMutex);
        } else {
            sched_yield();
        }
    }//pthread_exit(NULL);
    printf("Line %d finished\n", l);
}
void reallocateLuggage(int s){

}
int thereAreUnblockedSensors(){
    int i = 0, result = 0;
    for(i = 0; i < S; i++){
        pthread_mutex_lock(&sensor[i].mutex);
        printf("Acquired the mutex of sensor %d\n", i);
    }

    for (i = 0; i < S; i++){
        if(sensor[i].blocked == 0){
            result = 1; //there's at least one unblocked sensor
            break;
        }
    }
    for(i = S-1; i >= 0; i--){
        pthread_mutex_unlock(&sensor[i].mutex);
        printf("Realeased the mutex of sensor %d\n", i);
    }
    printf("return: %d\n", result);

    return result;
}

void * controlThread(){
    int s = 0, l = 0, locked = 0, i = 0;
    while(xrayedLuggage < N*M){
        s = getLeastBusySensor(999); //can also check if all sensors are blocked, returns -1 in this case
        if(s == -1){ //this means that all sensors are blocked, maintence time!
            printf("maintence time!\n" );
            pthread_cond_broadcast(&doMaintenceCond);
			printf("controlThread on maintenceCompleteBarrier\n");
            pthread_barrier_wait(&maintenceCompleteBarrier);//wait for maintence to complete
            printf("Aviso: Manutencao Final dos Sensores.\n");
        }
        i = (i + 1) % S;
        pthread_mutex_lock(&sensor[i].mutex);
        printf("controlThread sensor %d locked\n", i);
        pthread_mutex_lock(&reallocationMutex);
        if(sensor[s].count >= 10 && sensorToReallocateLuggage == -1){//this has to be 200
            sensorToReallocateLuggage = s;
        }
        pthread_mutex_unlock(&reallocationMutex);
        pthread_mutex_unlock(&sensor[i].mutex);
        printf("controlThread sensor %d unlocked\n", i);

    }

    //printf("\tAviso: Manutencao final dos sensores.\n");
    //pthread_exit(NULL);
}

void * xrayLuggage(void * id){

    pthread_barrier_wait(&startup);
    int s = (int) id; //get sensor index
    printf("Sensor %d started\n", s);

    while(xrayedLuggage < M*N){
        pthread_mutex_lock(&sensor[s].mutex);
        printf("sensor %d locked by its thread\n", s);
        while(sensor[s].blocked == 1){		//while this sensor is blocked
            printf("sensor %d waiting for maintence, mutex Realeased\n", s);
            pthread_cond_wait(&doMaintenceCond, &sensor[s].mutex); //wait for maintence..
			printf("sensor %d to execute maintence, mutex accquired\n", s);
            printf("Sensor %d: Manutencao\n", s);
            sensor[s].blocked = 0;
            sensor[s].count = 0;
			printf("Sensor %d on maintenceCompleteBarrier\n", s);
            //pthread_barrier_wait(&maintenceCompleteBarrier);//wait for every sensor to recieve the maintence
			printf("Sensor %d past maintenceCompleteBarrier\n", s);
            //wait for maintence to complete and unlock this sensor, then lock it again when maintence is complete
        }
        //wait(100); 		// analyze luggage for 0.5 s
        if(sensor[s].count < 10){
            pthread_mutex_lock(&sensorsMutex);
            printf("sensorsMutex locked by sensor %d thread\n", s);
            if (sensorsQueueOccupied[s] > 0){
                sensorsQueueOccupied[s]--;		//take luggage out of the queue
                sensor[s].count++;
                printf("sensor %d count size: %d\n", s, sensor[s].count);
                pthread_mutex_lock(&xrayedLuggageMutex); //since this is a global variable, a lock is necessary
                printf("xrayedLuggageMutex (%d) locked by sensor %d thread\n", xrayedLuggage , s);
                xrayedLuggage++;				//increment analyzed luggage count
                pthread_mutex_unlock(&xrayedLuggageMutex);
            }
            pthread_mutex_unlock(&sensorsMutex);
        } else {
			sensor[s].blocked = 1;
		}
        pthread_mutex_unlock(&sensor[s].mutex);
    }

    // while (doFinalMaintence == 0) {
    // 	printf("Sensor %d running\n", s);
    // 	while(xrayedLuggage < M*N){
    // 		if(sensor[s].blocked == 0){
    // 			if(sensor[s].count < 10){
    // 				pthread_mutex_lock(&sensorsMutex);
    // 				if(sensorsQueueOccupied[s] > 0){
    // 					//wait(0); //analyze luggage...
    // 					sensorsQueueOccupied[s] -= 1; //take luggage out.
    // 					pthread_mutex_lock(&xrayedLuggageMutex);
    // 					xrayedLuggage += 1; //increment total x-rayed luggage
    // 					pthread_mutex_unlock(&xrayedLuggageMutex);
    // 					//if(xrayedLuggage >= N*M*2/3)printf("xrayedLuggage count: %d\n", xrayedLuggage);
    // 				}
    // 				pthread_mutex_unlock(&sensorsMutex);
    // 			} else {
    // 				sensor[s].blocked = 1;
    // 			}
    // 		}
    // 	}
    // 	if(xrayedLuggage >= M*N){
    // 		doFinalMaintence = 1; //this makes sure all luggages were analyzed before the final
    // 	} else {
    // 		pthread_barrier_wait(&maintence);
    // 		printf("Sensor %d: Manutencao\n", s);
    // 		sensor[s].blocked = 0;
    // 		sensor[s].count = 0;
    // 		pthread_barrier_wait(&maintenceComplete);
    // 		printMessageOnce("Aviso: Todos os sensores receberam manutencao.\n\0"); //print this only once
    // 	}
    // }
    // pthread_barrier_wait(&maintence);
    // printf("Sensor %d: Manutencao\n", s);
    // pthread_barrier_wait(&maintenceComplete); //make sensors wait all other sensors finish their maintence
    // printMessageOnce("Aviso: Manutencao Final dos Sensores.\n\0"); //print this only once
    // pthread_exit(NULL);
    // printf("Sensor %d finished\n", s);
}

/***
*		@printMessageOnce(arg)
*		@description: Prints arg only once even though all sensor threads execute it;
***/
void printMessageOnce(char * message){
    int lockAquired = pthread_mutex_trylock(&maintenceMutex); //returns 0 if the lock was acquired
    if(lockAquired == 0){ // if the lock was acquired
        maintenceCount++;
        printf("%s%d\n", message, maintenceCount); //print message
    } //else.. just wait for all threads to try to acquire the lock
    pthread_barrier_wait(&maintenceMessage);	//this way only one sensor thread will print the above message.
    //I can reuse this barrier because all threads have already passed it before (thanks to maintenceComplete)
    pthread_mutex_unlock(&maintenceMutex);
}

void wait(int millisec){
    struct timespec ts;
    ts.tv_sec = millisec/1000; //quantity of seconds
    ts.tv_nsec = (millisec % 1000) * 1000000; // quantity of nanoseconds
    nanosleep(&ts, NULL); //wait for tv_sec seconds and tv_nsec nanoseconds
}

int main(int argc, char const *argv[]) {
    int i = 0;

    pthread_barrier_init(&maintence, NULL, S);
    pthread_barrier_init(&startup, NULL, S + N + 1);
    pthread_barrier_init(&maintenceMessage, NULL, S);
    pthread_barrier_init(&maintenceCompleteBarrier, NULL, S+1);

    pthread_create(&control, NULL, controlThread, NULL);
    for (i = 0; i < S; i++){
        sensorsQueueOccupied[i] = 0;
        pthread_create(&sensor[i].thread, NULL, xrayLuggage, (void *) i);
    }
    for (i = 0; i < N; i++) {
        pthread_create(&lines[i], NULL, sendLuggageToSensor, (void *) i);
    }
    pthread_barrier_wait(&startup);
    pthread_exit(NULL);
}
