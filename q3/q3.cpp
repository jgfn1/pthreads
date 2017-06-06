/***
	*
	* @author victoraurelio.com
	* @since 13/11/16
	*
*/
#include <bits/stdc++.h>

using namespace std;

// TO COMPILE:
    // g++ q3.cpp -std=c++0x -lpthread -o e
    // g++ q3.cpp -std=c++0x -pthread -o e

typedef struct request {
    int idClient;
    int idBuffer; // to check if exists in max 5 spaces at buffer.
    int pageContent;
    //condition_variable responseReady; // When server write page.
    //condition_variable responseFinished; // When client read the page.
    bool responseReady; // When server write page.
    bool responseFinished; // When client read the page.
    condition_variable resolve;
} Request;

/*

Um servidor da internet (representado por uma única thread) tem um buffer de 5 posições para
envio de páginas web a uma quantidade C de clientes que as requisitam. . Uma requisição só poderá
ser feita por um cliente quando o espaço no buffer estiver vazio. O cliente deverá informar ao servidor
quando a página for recebida, fazendo com que o servidor esvazie o buffer.

A saída deverá indicar quando um cliente receber uma página web.
               Saída:
                              CLIENTE 1 recebeu página 2
                              CLIENTE 2 recebeu página 4
                              CLIENTE 1 recebeu página 1
                              CLIENTE 3 recebeu página 10

*/

list<Request*> requests;
mutex canRequest; // only for client requests...
unique_lock<mutex> requestProcessing(canRequest);
unique_lock<mutex> newRequest(canRequest);
condition_variable createdRequest;
//condition_variable ableNewRequest;// to lock clients making to many requests...
bool canWriteRequest;
int maxPages, clientsLength;

void server();
void client(int id);
Request* make_request(int id);

int main(){
    // Allocating one thread for each line of matrix
    vector< thread > clients;
//    cin >> maxPages;
//    cin >> clientsLength;

    printf("uahehuae");

  /*  thread s = thread(server);
    for(int i=0; i<clientsLength; i++){
	clients.push_back( thread(client, i) );
    }*/

    pthread_exit(NULL);
}

/***
  *
  * @function server
  * @description
  *    This function build pages and serve on buffer.
  *
***/
void server(){
    
    int page = 0;
    while(true){
    	printf("Hi at server\n");
        // Safe zone:
	    createdRequest.wait(newRequest);// Waiting a client make a request.
	    canRequest.lock();

            Request *r = requests.front();// requests are processing in FIFO
            r->pageContent = page++;
            r->responseReady = true;
            r->resolve.notify_one();// WAKE UP THREAD TO READ ..
       

            // Acho que dá pra fazer assim aqui
            // canRequest.unlock e notificar o unique_lock para criar uma nova requisição..
            // -> assim enquanto a outra thread não lê outras threads podem requisitar.


            //r->resolve.wait(canRequest, []{return r->responseFinished});// Only remove request, after thread read data "returned";);
            r->resolve.wait(requestProcessing);// Only remove request, after thread read data "returned";);
            requests.pop_front();
            free(r); // free memory space.


            canWriteRequest = true;
            //ableNewRequest.notify_one(); // In case the a thread make one request and no have space..
            canRequest.unlock();

            if(page == maxPages){
                pthread_exit(NULL);
            }

    }

    pthread_exit(NULL);
}

/***
  *
  * @function cliente
  * @description
  *    This function get pages stored at buffer.
  *
***/
void client(int id){

   while(1){
    	printf("Hi i'm client %d \n", id);
          canRequest.lock();
          Request* r;
          while(requests.size() > 4){// Impossible have 5 requests... is buffer size!
              //ableNewRequest.wait(requestProcessing, ableNewRequest);
          }
          r = make_request(id);
          requests.push_back(r);
	  createdRequest.notify_one(); // or notify_one ? makes server wakeup..

          printf("I'm thread %d requesting new page at buffer[%d]\n", r->idClient,  r->idBuffer);

          while(r->pageContent == -1){
              //r->resolve.wait(canRequest, []{return r->responseReady});
              r->resolve.wait(requestProcessing);
          }

          printf("I'm thread %d reading page: %d at buffer[%d]\n", r->idClient, r->pageContent, r->idBuffer);

          r->resolve.notify_one();
          canRequest.unlock();

    }
    pthread_exit(NULL);
}

Request* make_request(int id){
    Request *r = (Request*) malloc(sizeof(Request));
    r->idClient = id;
    r->idBuffer = requests.size() - 1;
    r->pageContent = -1;
    r->responseReady = false;
    r->responseFinished = false;

    return r;
}

/***
  *
  * @function servidor
  * @description
  *    This function build pages and serve on buffer.
  *
***/
/*void* servidor(){
      while(1){
          int i;
          pthread_mutex_lock(&mutex);
          while(requests.size() == 0) pthread_cond_wait(&pageRequested, &mutex);
          for(i=0; i<BUFFER_SIZE; i++) buffer[i] = indexPageProducer++; // Seta o número de página que está a ser produzida

          bufferNitens = BUFFER_SIZE;  // Buffer não está vazio!

          //printf("Servidor vai fazer %d páginas: \n", BUFFER_SIZE);
          for(i=0; i<BUFFER_SIZE; i++) printf("\t%d\n", buffer[i]);

          //printf("Páginas produzidas:  %d\n", bufferNitens);

          if(bufferNitens == BUFFER_SIZE){// Irá disparar o buffer cheio, e liberar o mutex
              pthread_cond_broadcast(&bufferFill);
              pthread_mutex_unlock(&mutex);
          }
      }
}
*/
/***
  *
  * @function cliente
  * @description
  *    This function get pages stored at buffer.
  *
***/
/*void* cliente(int id){
      while(1){
          pthread_mutex_lock(&mutex);
          while(bufferNitens == 0) pthread_cond_wait(&bufferFill, &mutex);

          printf("CLIENTE %d recebeu a página %d\n", *id, buffer[indexPageConsumer % 5]);
          bufferNitens--;
          indexPageConsumer++;

          if(bufferNitens == 0){
              pthread_cond_signal(&pageRequested);
          }
          pthread_mutex_unlock(&mutex);
      }
      //pthread_exit(NULL);
}*/
