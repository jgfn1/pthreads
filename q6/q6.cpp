/***
	*
	* @author victoraurelio.com
	* @since 13/11/16
	*
*/
#include <bits/stdc++.h>
#include <unistd.h>
// #include "stdc++.h"

#define MAX_LUGGAGES_AT_QUEUE 5
#define MAX_SENSOR_LUGGAGES 200
#define TRACK_LUGGAGES 300

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
void* newIndex(int id);
void putInQueue(Queue *q, int luggages);
Queue * getQueue();
Queue *makeQueue(int id);
pthread_t * makeThread();



int main(){
	// Allocating one thread for each line of matrix
  int nt, ns;
  pthread_t* tmpThread;
  Queue* tmpQueue;
	vector< pthread_t* > ttracks;
  vector< pthread_t* > tsensors;
  cin >> nt >> ns;

  amountEnableSensors = ns;

  // Adding luggages in queues
  for(int i=0; i<ns; i++){
      tmpQueue = makeQueue(i);
      AirportQueues.push_back(tmpQueue);
      AirportQueuesInOrder.push(tmpQueue);
  }
  // Starting sensors
  for(int i=0; i<ns; i++){
      tmpThread = makeThread();
      if(pthread_create(tmpThread, NULL, sensor, newIndex(i)) == 0){
          tsensors.push_back(tmpThread);
      }
  }
  // Starting tracks
  for(int i=0; i<nt; i++) {
      tmpThread = makeThread();
      if(pthread_create(tmpThread, NULL, track, newIndex(i)) == 0){
          ttracks.push_back(tmpThread);
      }
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
  int *dd = (int*) arg;
  int id = *dd;

  int trackItens = TRACK_LUGGAGES;

  while(trackItens > 0){
    	Queue *shortQueue = getQueue();
      if(shortQueue != NULL){
          //printf("[ESTEIRA %d]: colocando na fila %d \n",id, shortQueue->id);
        	// lock was happend at getQUeue lock(&shortQueue->mtxCanUseQueue);
        	while(shortQueue->size == MAX_LUGGAGES_AT_QUEUE){
        		condWait(&shortQueue->cndQueueEmpty, &shortQueue->mtxCanUseQueue);
        	}

          shortQueue->size++;
          trackItens--;

          if(trackItens == 0){
              printf("[ESTEIRA %d]: coloquei na fila %d me restam [%d]\n", id, shortQueue->id, trackItens);
          }
          signal(&shortQueue->cndQueueFill);
          unlock(&shortQueue->mtxCanUseQueue);
      }
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
  int *dd = (int*) arg;
  int id = *dd;

	int amountLuggagePassed = 0;

	while(true){
  		Queue *myQueue = AirportQueues[id];
      /*if(myQueue == NULL){
          printf("fucking_crazyy\n");
          exit(1);
      }*/
  		lock(&myQueue->mtxCanUseQueue);
  		while(myQueue->size == 0){
  			   signal(&myQueue->cndQueueEmpty); // Acordará alguma esteira se estiver bloqueada para escrever na queue...
  			   condWait(&myQueue->cndQueueFill, &myQueue->mtxCanUseQueue); // Sempre que alguma esteira escrever na queue ela dará siganl para o mtxQueueFIlll dentro da fila
  		}
      //TSTprintf("[SENSOR %d]:  Passing luggage %d!!!\n", id, myQueue->size);

      // Ler uma Mala
  		myQueue->size--;
      amountLuggagePassed++;
  		usleep(1);
      if(myQueue->size < 0){
          printf("pqp \n");
      }
  		if(amountLuggagePassed == MAX_SENSOR_LUGGAGES){// Se liga na porra da manutenção...
                      printf("!!!! [SENSOR %d]: going to maintenance !!! %d\n", id, myQueue->size);
                			Queue *t = getQueue();
                			if(t != NULL && t->id != -1){
                         //TSTprintf(" _________ REALLOC LUGAGGES _________ \n");
                				 putInQueue(t, myQueue->size);
                				// Será que vai caber tudo? tenho que mandar um por um ensse carai

                			}

                			amountEnableSensors--;
                      if(amountEnableSensors == 0){
                          printf(" *******  !!!!  ALL SENSORS GOING TO maintenance !!!!! ******** \n");
                    			//barrierWait(A_BARREIRA_DE_MANUTENCAO);

                    			amountEnableSensors++;
                    			amountLuggagePassed = 0;
                      }
  		}else{
          //TSTprintf("[SENSOR %d]: passed to me a laggage\n", id);
    			unlock(&myQueue->mtxCanUseQueue); // Para que minha fila já possa ser usada por alguma esteira..
  		}

      signal(&myQueue->cndQueueEmpty);
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
  int size = AirportQueuesInOrder.size();

	for(int i=0; i<size && q == NULL; i++){
		queueTMP = AirportQueuesInOrder.top();
    if(queueTMP != NULL){
        if(tryLock(&queueTMP->mtxCanUseQueue)){
            q = queueTMP;
        }
    		tmp.push_back(queueTMP);
    		AirportQueuesInOrder.pop();
    }
	}

  // "RECARREGANDO A priority_queue da pior forma a queues in order"
  while(!tmp.empty()){
      AirportQueuesInOrder.push(tmp.back());
      tmp.pop_back();
  }

  //printf("\t[get_queue]Returning to queue %d laggage %d [ %d == %d ]\n", q->id, q->size, AirportQueues.size(), AirportQueuesInOrder.size());
	unlock(&mtxAirportQueues);
	return q;
}

void putInQueue(Queue *q, int luggages){
  printf("[WARN] at put in queue %d total of %d luggages [WARN]\n", q->id, luggages);
	//lock(&mtxAirportQueues);
	//lock(&q->mtxCanUseQueue);

	q->size += luggages;

  printf("New size %d\n", q->size);

  // "atualizando da pior forma a queues in order"
  Queue refresh;
  refresh.size = -1;
  AirportQueuesInOrder.push(&refresh);// Adding refresh

  AirportQueuesInOrder.pop();// Will remove refresh, because resfresh is smallest size...

  //make_heap(const_cast<Queue**> (&AirportQueuesInOrder.top()), const_cast<Queue**>(&AirportQueuesInOrder.top()) + AirportQueuesInOrder.size(), CompareQueue());

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

void* newIndex(int id){
  int *p = (int*) malloc(sizeof(int));
  *p = id;
  return p;
}


pthread_t * makeThread(){
    pthread_t *t = (pthread_t*) malloc (sizeof(pthread_t));
    return t;
}
