#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <limits.h>

#define B 100000
#define P 10
#define C 10

typedef struct elem{
   int value;
   struct elem *prox;
}Elem;

typedef struct blockingQueue{
   unsigned int sizeBuffer, statusBuffer;
   Elem *head,*last;
}BlockingQueue;

pthread_t producers[P], consumers[C];

pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t emptyCond = PTHREAD_COND_INITIALIZER, fillCond = PTHREAD_COND_INITIALIZER;

/***
  *
  * @function newBlockingQueue
  *
  * @description Creates a blocking queue with the size of SizeBuffer.
  *
***/

BlockingQueue * newBlockingQueue(unsigned int SizeBuffer){
	BlockingQueue * queue = (BlockingQueue *) calloc(1, sizeof(BlockingQueue));
	//calloc sempre zera tudo, sem necessidade de zerar qualquer variável.
	queue->sizeBuffer = SizeBuffer;
	return queue;
}

/***
  *
  * @function putBlockingQueue(BlockingQueue* Q, int newValue)
  *
  * @description Puts in a the value represented by newValue in the queue. If
  * the queue is full this function blocks the thread and waits until the buffer
  * has any spaces;
  *
***/

void putBlockingQueue(BlockingQueue* Q, int newValue){
	//the new value is the id of the thread
	pthread_mutex_lock(&queueMutex);

	while (Q->statusBuffer == Q->sizeBuffer){
		printf("Producer thread is sleeping: queue is full.\n");
		pthread_cond_wait(&fillCond, &queueMutex); //wait for permission to fill the queue
	}
	//create element to put in the queue
	Elem * elem = (Elem *) malloc(sizeof(Elem));

	//initilize element
	elem->value = newValue;
	elem->prox = NULL;

	//put element in the queue
	if(Q->head == NULL){
		Q->head = elem;
		Q->last = elem;
	} else {
		Q->last->prox = elem;
		Q->last = Q->last->prox;
	}
	Q->statusBuffer++;
	pthread_cond_signal(&emptyCond);
	pthread_mutex_unlock(&queueMutex);
}

/***
  *
  * @function takeBlockingQueue(BlockingQueue* Q)
  *
  * @description Removes the last element from the queue pointed by Q and also
  * returns its value.
  *
***/

int takeBlockingQueue(BlockingQueue* Q){
	pthread_mutex_lock(&queueMutex);
	int value;
	Elem * elem;
	while (Q->statusBuffer == 0){
		printf("Consumer thread is sleeping: queue is empty.\n");
		pthread_cond_wait(&emptyCond, &queueMutex); //wait for permission to empty the queue
	}
	elem = Q->head;
	value = Q->last->value;

	if(Q->head == Q->last){
		Q->head = NULL;
		Q->last = NULL;
	} else {
		while(elem->prox != Q->last){
			elem = elem->prox;
		}
		free(elem->prox);
		elem->prox = NULL;
		Q->last = elem;
	}
	Q->statusBuffer--;
	pthread_cond_signal(&fillCond);
	pthread_mutex_unlock(&queueMutex);
	return value;
}

/***
  *
  * @function producer(void * q)
  *
  * @description Produces numbers puts them into the BlockingQueue pointed by
  * the variable 'q'.
  *
***/

void * producer (void * q){
	BlockingQueue * queue = (BlockingQueue *) q;
	unsigned int value;
	while (1) {
		value = rand() % UINT_MAX; // a random number between 0 and the max value of an unsigned int minus one.
		putBlockingQueue(queue, value);
	}
}

/***
  *
  * @function consumer(void * q)
  *
  * @description Consumes numbers from the BlockingQueue pointed by the variable
  * 'q' by removing the last element of the queue and (should be) using its value.
  *
***/

void * consumer (void * q){
	BlockingQueue * queue = (BlockingQueue *) q;
	while (1) {
		takeBlockingQueue(queue); //nothing to do with the number.. nothing in the specification of the problem.
	}
}

int main(int argc, char const *argv[]) {
	int i = 0;

	BlockingQueue * queue = newBlockingQueue(B);

	for(i = 0; i < P; i++){
		pthread_create(&producers[i], NULL, producer,(void *) queue);
	}

	for(i = 0; i < P; i++){
		pthread_create(&consumers[i], NULL, consumer,(void *) queue);
	}
	pthread_exit(NULL);
}
