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
int xrayedLuggageCount = 0;

pthread_t line[N];

int getLeastBusySensor(int caller){
	int i = 0, result = 0;
	printf("GLBS Caller: %d\n", caller);

	for(i = 0; i < S; i++){ //lock all sensors
        pthread_mutex_lock(&sensor[i].mutex);
        printf("GLSB: Caller %d acquired sensor %d \n", caller, i);
    }

	for (i = 0; i < S-1; i++){
		if(sensor[i].queue < sensor[i+1].queue)
			result = i;
		else
			result = i+1;
	}

	for(i = S-1; i >= 0; i--){ //lock all sensors
         if (i != result) pthread_mutex_unlock(&sensor[i].mutex);
        printf("GLSB: Caller %d realeased sensor %d \n", caller, i);
    }
	return result;
}

void * xrayLuggage(void * s){
	int this = (int) s; //this sensor's index
	printf("sensor %d started\n", this);
	int i;

	while (xrayedLuggageCount < N*M){
		pthread_mutex_lock(&sensor[this].mutex);
		if(sensor[this].queue > 0){
			sensor[this].queue--;
			sensor[this].count++;
			pthread_mutex_lock(&xrayedLuggageMutex);
			xrayedLuggageCount++;
			pthread_mutex_unlock(&xrayedLuggageMutex);
			printf("sensor: %d, count: %d, queue: %d, total count: %d\n", this, sensor[this].count, sensor[this].queue, xrayedLuggageCount);
		}
		pthread_mutex_unlock(&sensor[this].mutex);
	}
}

void * sendLuggageToSensor(void * line){
	int this = (int) line;
	printf("line %d started\n", this);
	int remainingLuggage = M;
	int s = 0, i;
	while (remainingLuggage > 0) {
		s = getLeastBusySensor(this); //mutex still locked
		//no need to lock sensor[s] because it's still locked from the lsb
		printf("line %d, LBS: %d, lock acquired\n", this, s);
		if(sensor[s].queue < 5){
			sensor[s].queue++;
			remainingLuggage--;
			printf("Line %d: sensor[%d].queue = %d, remaining: %d\n", this, s, sensor[s].queue, remainingLuggage);
		}
		pthread_mutex_unlock(&sensor[s].mutex);
		sched_yield();
	}
}

int main(int argc, char const *argv[]) {
	int i;
	for(i = 0; i < N; i++){
		pthread_create(&line[i], NULL, sendLuggageToSensor, (void *) i);
	}
	for(i = 0; i < S; i++){
		pthread_create(&sensor[i].thread, NULL, xrayLuggage, (void*) i);
	}
	pthread_exit(NULL);
}
