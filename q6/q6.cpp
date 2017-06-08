/***
	*
	* @author victoraurelio.com
	* @since 13/11/16
	*
*/
#include <bits/stdc++.h>

#define MAX_QUEUE_SIZE 5
#define MAX_LUGGAGE 10

using namespace std;

// TO COMPILE:
	// g++ q6.cpp -std=c++0x -lpthread -o bin
	// g++ q6.cpp -std=c++0x -pthread -o bin

// TO EXECUTE:
	// ./e < in


// Queue Structures and ....
  typedef struct Queue{
	  int id;
	  int size;
	  pthread_mutex_t mtxCanUseQueue;
	  pthread_cond_t cndQueueEmpty, cndQueueFill;
  } Queue ;

priority_queue<Queue> AirportQueuesInOrder;
vector<Queue> AirportQueues; // each index is ID of sensor.
pthread_mutex_t mtxAirportQueues;


// Variáveis do buffer de realocação
int bufferRealloc;
pthread_mutex_t mtxBufferRealloc;
pthread_cond_t bufferFill, bufferEmpty;

// Variáveis do sensor
int amountEnableSensors;

void lock(pthread_mutex_t * mutexAddr){
	pthread_mutex_lock(mutexAddr);
}

void unlock(pthread_mutex_t * mutexAddr){
	pthread_mutex_unlock(mutexAddr);
}

void condWait(pthread_cond_t * condAddr, pthread_mutex_t * mutexAddr){
	pthread_cond_wait(condAddr, mutexAddr);
}

void barrierWait(pthread_barrier_t * barrierAddr){
	pthread_barrier_wait(barrierAddr);
}

int main(){
	// Allocating one thread for each line of matrix
	vector< thread > ;
}

/***
  *
  * @function esteira/trilha
  * @description
  *	This function build pages and serve on buffer.
  *
***/
void* track(){

	Queue shortQueue = getQueue();

	lock(shortQueue->mtxCanUseQueue);

	while(shortQueue->size == MAX_QUEUE_SIZE){
		wait(shortQueue->cndQueueEmpty);
	}



	pthread_exit(NULL);
}

/***
  *
  * @function sensor
  * @description
  *	...
  *
***/

void* sensor(int id){
	int amountLuggagePassed = 0;
	while(true){
		Queue *myQueue = AirportQueues[id];

		lock(myQueue->mtxCanUseQueue);
		while(myQueue->size == 0){
			signal(myQueue->mtxQueueEmpty); // Acordará alguma esteira se estiver bloqueada para escrever na queue...
			pthread_cond_wait(myQueue->mtxQueueFill); // Sempre que alguma esteira escrever na queue ela dará siganl para o mtxQueueFIlll dentro da fila
		}

		// Ler uma Mala
		myQueue->size --;
		sleep(0.5);

		// Se liga na porra da manutenção...
		if(counterExecution == MAX_LUGGAGE){
			Queue *t;
			if(t = getQueue() && t->id == -1){
				putInQueue(myQueue, myQueue->size);
				// Será que vai caber tudo? tenho que mandar um por um ensse carai
			}

			amountEnableSensors--;
			barrierWait(A_BARREIRA_DE_MANUTENCAO);
			amountEnableSensors++;
			amountLuggagePassed = 0;
		}else{
			unlock(myQueue->mtxCanUseQueue); // Para que minha fila já possa ser usada por alguma esteira..
		}
	}
	pthread_exit(NULL);
}

/*
	Objetivo:
		Consultar a disponibilidade de inserir uma mala em uma fila, dando prioridade as filas de menor tamanho.

	Caminho:
		Damos no lock na priority_queue de filas do aeroporto e retiramos item por item, até que seja possível dar lock na fila
		if(foi possivel dar lock na fila){
			Então retornaremos a fila e ela está locked ou seja em alguma execução paralela de
						getqueue ele nao conseguirá retirar ou inserir um item dessa fila.
		}
*/
Queue * getQueue(){
	lock(mtxAirportQueues);

	priority_queue<Queue> tmp;
	Queue *q, *queueTMP;
	for(int i=0; i<AirportQueues.size() && lugage->id == -1; i++){
		queueTMP = AirportQueues.front();
		// try lock AirportQueues.front(){
			// queue = AirportQueues.front();
		//}
		tmp.push(queueTMP);
		AirportQueues.pop();
	}

	unlock(mtxAirportQueues);
	return q;
}

void putInQueue(Queue *q, int elements){
	lock(mtxAirportQueues);
	lock(q->mtxQueue);

	q->size += elements;
	// Refazendo a priority_queue;
	priority_queue<Queue, AirportQueues, CompareQueue> AirportQueuesInOrder;

	// Refazendo a priority_queue;
	priority_queue<Queue, vector<Queue>, CompareQueue> AirportQueuesInOrder;
	//AirportQueuesInOrder = t; //wtf is 't'?

	unlock(q->mtxQueue);
	unlock(mtxAirportQueues);
}

class CompareQueue {
	bool operator() (Queue *a, Queue *b){
		return a->size > b->size;
	}
};
