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
#define N 4
#define M 2
#define S 1

typedef
struct Sensor{
	pthread_t thread;
	int blocked; //denotes if the sensor is blocked or not (i.e. to know if it's possible not to put any luggage in its queue)
	int count; //counts how many luggages where checked. (for maintance...)
	pthread_mutex_t mutex; //mutex to add or remove luggages from queue
} Sensor;

//with respect to sensors
Sensor sensor[S] = { {(pthread_t) NULL, 0, 0, PTHREAD_MUTEX_INITIALIZER} };
int sensorQueueSize[S];
pthread_cond_t maintenceCompleteCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t sensorQueueSizeMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reallocateMutex = PTHREAD_MUTEX_INITIALIZER;
int analyzedLuggageCount = 0;

//with respect to lines
pthread_t line[N];

//with respect to the control thread
pthread_t control;
pthread_mutex_t lineBufferMutex[N] = { PTHREAD_MUTEX_INITIALIZER };
pthread_cond_t fillLineBuffer[N] = { PTHREAD_COND_INITIALIZER };
int lineBuffer[N];
int reallocationBuffer;

void wait(int millisec){
	struct timespec ts;
	ts.tv_sec = millisec/1000; //quantity of seconds
	ts.tv_nsec = (millisec % 1000) * 1000000; // quantity of nanoseconds
	nanosleep(&ts, NULL); //wait for tv_sec seconds and tv_nsec nanoseconds
}

void sendLuggageToSensor(int s){
	//printf("sendLuggageToSensor %d, try sensor's lock\n", s);
	pthread_mutex_lock(&sensor[s].mutex);
	//printf("sendLuggageToSensor %d, done\n", s);
	if(sensor[s].count < 2)
		sensor[s].count++;
	// if(sensor[s].count == 2){ //if this sensor has analyzed its 200th luggage right now
	// 	sensor[s].blocked = 1; //block sensor
	// 	//printf("sendLuggageToSensor %d, reallocateMutex lock\n", s);
	// 	pthread_mutex_lock(&reallocateMutex);
	// 	//printf("sendLuggageToSensor %d, got it\n", s);
	// 	reallocationBuffer += sensorQueueSize[s]; //send this sensor's luggage to the reallocation buffer
	// 	pthread_mutex_unlock(&reallocateMutex);
	// 	sensorQueueSize[s] = 0; //empty this sentors queue..
	// } //PS: this can't be an "else" because I don't want to have failure in sending luggage to sensors
	pthread_mutex_unlock(&sensor[s].mutex);
}

int getLeastBusySensor(){
	int i = 0, leastBusySensor = -1;

	if(S == 1){
		//printf("#############Realeased by getLeastBusySensor\n");
		if (sensor[0].blocked) return -1;
		else return 0;
	}

	pthread_mutex_lock(&sensorQueueSizeMutex); //
	//printf("#############owned by getLeastBusySensor\n");

	for(i = 0; i < S; i++){ //lock all sensors
		pthread_mutex_lock(&sensor[i].mutex);

	}



	for(i = 0; i < S - 1; i++){
		if(sensor[i].blocked == 0){ //if the first sensor is not blocked
			if(sensor[i+1].blocked == 0){ //and the second sensor is not blocked
				if(sensorQueueSize[i] < sensorQueueSize[i+1])//compare the size of their queues and select the queue with the lower size
					leastBusySensor = i;
				else
					leastBusySensor = i + 1;
			} else {
				leastBusySensor = i; //the least busy sensor will be the one not blocked
			}
		}//if not, just go on to the next test (iteration)
	}

	for(i = S-1; i >= 0; i--){//unlock all sensors
		pthread_mutex_unlock(&sensor[i].mutex);
	}
	pthread_mutex_unlock(&sensorQueueSizeMutex);
	//printf("#############Realeased by getLeastBusySensor\n");

	return leastBusySensor; //if all sensors are blocked, -1 will be returned
}

void * linesThread(void * id){
	int l = (int) id;
	int remaining = M;
	while(remaining > 0){
		pthread_mutex_lock(&lineBufferMutex[l]);		//lock this line's buffer (in the control thread)

		while(lineBuffer[l] == 1){		//while the buffer is full...
			pthread_cond_wait(&fillLineBuffer[l], &lineBufferMutex[l]); //wait command to fill buffer
		}

		lineBuffer[l] = 1; //fill this line's buffer
		remaining--;//decrement remaining luggage for this line
		printf("line: %d, remaining: %d\n", l, remaining);

		pthread_mutex_unlock(&lineBufferMutex[l]);		//unlock this line's buffer (in the control thread)
	}
}

void * sensorsThread(void * id){
	int s = (int) id;
	while(analyzedLuggageCount < N*M){		//while it's not time for the final maintence
		pthread_mutex_lock(&sensor[s].mutex);	//lock this sensor
		////printf("#############owned by sensorThread %d\n", s);
		while(sensor[s].blocked == 1){		//while this senor is blocked
			pthread_cond_wait(&maintenceCompleteCond, &sensor[s].mutex);
			//wait for maintence to complete and unlock this sensor, then lock it again when maintence is complete
		}
		//wait(100); 		// analyze luggage for 0.5 s

		pthread_mutex_lock(&sensorQueueSizeMutex);
		printf("#############owned by sensorThread %d\n", s);
		if (sensorQueueSize[s] > 0){
			sensorQueueSize[s]--;	//take luggage out of the queue
			analyzedLuggageCount++;	//increment analyzed luggage count
		} else {
			sched_yield();
		}
		pthread_mutex_unlock(&sensorQueueSizeMutex);
		//printf("#############Realeased by sensorThread %d\n", s);
		pthread_mutex_unlock(&sensor[s].mutex);
	}
}

void * controlThread(){
	int s = 0, l = 0, locked = 0, i = 0;
	while(analyzedLuggageCount < N*M){
		s = getLeastBusySensor();
		if(s == -1){ //this means that all sensors are blocked, maintence time!

			for( i = 0; i < S; i++){
				printf("Sensor %d: Manutencao\n", i);
				sensor[s].blocked = 0;
			}
			pthread_cond_broadcast(&maintenceCompleteCond);
			//printf("\tAviso: Todos os sensores receberam manutencao.\n");
			s = 0; //all sensors are unnocupied so I might as well get the first
		}
		pthread_mutex_lock(&sensorQueueSizeMutex);
		//printf("#############owned by controlThread\n");
		if (sensorQueueSize[s] < 5){ //if the least busy sensor has space for one me item in it's queue
			if(reallocationBuffer > 0){
				sendLuggageToSensor(s);
				sensorQueueSize[s]++;
				reallocationBuffer--;
			} else { //in case there's no luggage to reallocate, get one from the lineBuffer
				do{ //tries to lock a a line's buffer until it succeeds
					l = (l + 1) % N; //get the next line number(between 0 and N - 1).
					//printf(" trylock line %d\n", l);
					locked = pthread_mutex_trylock(&lineBufferMutex[l]); //try to lock this line's buffer
					if(lineBuffer[l] == 0 && locked == 0){ //if there is nothing in the buffer
						pthread_mutex_unlock(&lineBufferMutex[l]); //unlock the mutex
						locked = 1; // try to get the next mutex
						sched_yield();
					} else{
						//printf("success\n");
					}
				} while(locked == 0 && analyzedLuggageCount < N*M); //if I can't get the lock, try the next buffer. And prevent infinite loop in case all work is done
				sendLuggageToSensor(s);
				lineBuffer[l]--; //empty this line's buffer
				sensorQueueSize[s]++;
				pthread_cond_signal(&fillLineBuffer[l]); //signal for the thread to fill its buffer
				pthread_mutex_unlock(&lineBufferMutex[l]);
			}
		} else {
			sched_yield(); //make another thread run because all sensors' queues are fully occupied
			printf("sensor %d full\n", s);
		}
		pthread_mutex_unlock(&sensorQueueSizeMutex);
		//printf("#############Realeased by controlThread\n");
	}
	for(i = 0 ; i < S; i++){
		printf("Sensor %d: Manutencao\n", i);
		sensor[s].blocked = 0;
	}
	pthread_cond_broadcast(&maintenceCompleteCond);
	//printf("\tAviso: Manutencao final dos sensores.\n");
	//pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {
	int i = 0;
	for(i = 0; i < N; i++){
		pthread_create(&line[i], NULL, linesThread, (void *) i);
	}
	pthread_create(&control, NULL, controlThread, NULL);
	for(i = 0; i < N; i++){
		pthread_create(&sensor[i].thread, NULL, sensorsThread, (void*) i);
	}
	pthread_exit(NULL);
}
