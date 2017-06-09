/***
	*
	* @author victoraurelio.com
	* @since 13/11/16
	*
*/
#include <bits/stdc++.h>
#include <unistd.h>
// #include "stdc++.h"

#define MAX_QUEUE_SIZE 5
#define MAX_LUGGAGE 10
#define TRACK_SIZE 10

using namespace std;

// TO COMPILE:
	// g++ q6.cpp -std=c++0x -lpthread -o bin
	// g++ q6.cpp -std=c++0x -pthread -o bin

// TO EXECUTE:
	// ./e < in

// Queue Structures and ....
typedef struct queue{
  int id;
  int size;
  pthread_mutex_t mtxCanUseQueue;
  pthread_cond_t cndQueueEmpty, cndQueueFill;
} Queue ;

class CompareQueue {
	public:
      bool operator() (Queue *a, Queue *b){
  		    return a->size > b->size;
  	  }
};



vector<Queue*> AirportQueues; // each index is ID of sensor.
priority_queue<Queue*, vector<Queue*>, CompareQueue> AirportQueuesInOrder;
pthread_mutex_t mtxAirportQueues;


// Variáveis do buffer de realocação
int bufferRealloc;
pthread_mutex_t mtxBufferRealloc;
pthread_cond_t bufferFill, bufferEmpty;

// Variáveis do sensor
int amountEnableSensors;

// Variáveis para manutenção
//pthread_barrier_t barrierMaintenance;

void lock(pthread_mutex_t * mutexAddr){ pthread_mutex_lock(mutexAddr); }
void unlock(pthread_mutex_t * mutexAddr){ pthread_mutex_unlock(mutexAddr); }
void condWait(pthread_cond_t * condAddr, pthread_mutex_t * mutexAddr){ pthread_cond_wait(condAddr, mutexAddr); }
void signal(pthread_cond_t *condAddr){ pthread_cond_signal(condAddr); }
bool tryLock(pthread_mutex_t * mutexAddr){ return pthread_mutex_trylock(mutexAddr) == 0; }
//void barrierWait(pthread_barrier_t * barrierAddr){ pthread_barrier_wait(barrierAddr); }
void* track(void* arg);
void* sensor(void* arg);
void* newIndex(int id){
  int *p = (int*) malloc(sizeof(int));
  *p = id;
  printf("Making %d\n", *p);
  return p;
}
void putInQueue(Queue *q, int elements);
Queue * getQueue();
Queue *makeQueue(int id);
pthread_t * makeThread();



int main(){
	// Allocating one thread for each line of matrix
  int nt, ns,i,j;
  pthread_t* tmpThread;
  Queue* tmpQueue;
	vector< pthread_t* > ttracks;
  vector< pthread_t* > tsensors;
  cin >> nt >> ns;

  for(i=0; i<ns; i++){
      tmpThread = makeThread();
      tmpQueue = makeQueue(i);

      if(pthread_create(tmpThread, NULL, sensor, newIndex(i)) == 0){
          // Adding thread
          tsensors.push_back(tmpThread);
          // Adding queues to this sensor
          AirportQueues.push_back(tmpQueue);
          AirportQueuesInOrder.push(tmpQueue);
      }

  }
  for(int j=0; j<nt; j++) {
      tmpThread = makeThread();
      if(pthread_create(tmpThread, NULL, track, NULL) == 0)
          ttracks.push_back(tmpThread);
  }

  pthread_exit(NULL);
}

/***
  *
  * @function esteira/trilha
  * @description
  *	This function build pages and serve on buffer.
  *
***/
void* track(void* arg){
  int trackItens = TRACK_SIZE;

  while(trackItens > 0){
    	Queue *shortQueue = getQueue();

    	lock(&shortQueue->mtxCanUseQueue);

    	while(shortQueue->size == MAX_QUEUE_SIZE){
    		condWait(&shortQueue->cndQueueEmpty, &shortQueue->mtxCanUseQueue);
    	}

      printf(" Track X removing item.\n");
      shortQueue->size++;
      trackItens--;

      unlock(&shortQueue->mtxCanUseQueue);
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

void* sensor(void* arg){//int id
  //cout << " I'm sensor " << id << endl;
  //printf("uhaehuaeha\n");
  int *dd = (int*) arg;
  int id = *dd;
  //printf("truth\n");
	int amountLuggagePassed = 0;
  //pthread_exit(NULL);
	while(true){
    printf("Sensor id: %d\n", id);
		Queue *myQueue = AirportQueues[id];
    printf("My queue is: %d\n", myQueue->id);
		lock(&myQueue->mtxCanUseQueue);
		while(myQueue->size == 0){
			signal(&myQueue->cndQueueEmpty); // Acordará alguma esteira se estiver bloqueada para escrever na queue...
			condWait(&myQueue->cndQueueFill, &myQueue->mtxCanUseQueue); // Sempre que alguma esteira escrever na queue ela dará siganl para o mtxQueueFIlll dentro da fila
		}

		// Ler uma Mala
		myQueue->size --;
		usleep(500);

		// Se liga na porra da manutenção...
		if(amountLuggagePassed == MAX_LUGGAGE){
			Queue *t;
			if(t = getQueue(), t->id == -1){
				putInQueue(myQueue, myQueue->size);
				// Será que vai caber tudo? tenho que mandar um por um ensse carai
			}

			amountEnableSensors--;

			//barrierWait(A_BARREIRA_DE_MANUTENCAO);
			amountEnableSensors++;
			amountLuggagePassed = 0;
		}else{
			unlock(&myQueue->mtxCanUseQueue); // Para que minha fila já possa ser usada por alguma esteira..
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
	lock(&mtxAirportQueues);

	vector<Queue*> tmp;
	Queue *q, *queueTMP;
  q = NULL;
  queueTMP = AirportQueuesInOrder.top();
	for(int i=0; i<AirportQueuesInOrder.size() && q == NULL; i++){
		queueTMP = AirportQueuesInOrder.top();
    if(tryLock(&queueTMP->mtxCanUseQueue)){
        // mutex has been locked..
        q = queueTMP;
    }
		tmp.push_back(queueTMP);
		AirportQueuesInOrder.pop();
	}

  // "atualizando da pior forma a queues in order"
  while(!tmp.empty()){
      AirportQueuesInOrder.push(tmp.back());
      tmp.pop_back();
  }

	unlock(&mtxAirportQueues);
	return q;
}

void putInQueue(Queue *q, int elements){
	lock(&mtxAirportQueues);
	lock(&q->mtxCanUseQueue);

	q->size += elements;

  // "atualizando da pior forma a queues in order"
  /*Queue refresh;
  refresh.size = -1;
  AirportQueuesInOrder.push(&refresh);// Adding refresh
  AirportQueuesInOrder.pop();// Will remove refresh, because resfresh is smallest size...
*/
  make_heap(const_cast<Queue**> (&AirportQueuesInOrder.top()), const_cast<Queue**>(&AirportQueuesInOrder.top()) + AirportQueuesInOrder.size(), CompareQueue());

	unlock(&q->mtxCanUseQueue);
	unlock(&mtxAirportQueues);
}

Queue *makeQueue(int id){
    Queue *q = (Queue*) malloc (sizeof(Queue));
    q->id = id;
    q->size = 0;
    //q->mtxCanUseQueue = PTHREAD_MUTEX_INITIALIZER;
    return q;
}

pthread_t * makeThread(){
    pthread_t *t = (pthread_t*) malloc (sizeof(pthread_t));
    return t;
}
