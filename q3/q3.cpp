/***
	*
	* @author victoraurelio.com
	* @since 13/11/16
	*
*/
#include <bits/stdc++.h>

#define BUFFER_SIZE 5

using namespace std;

// TO COMPILE:
    // g++ q3.cpp -std=c++0x -lpthread -o e
    // g++ q3.cpp -std=c++0x -pthread -o e

// TO EXECUTE:
    // ./e < in 

typedef struct request {
    int idClient;
    int idBuffer; // to check if exists in max 5 spaces at buffer.
    int pageContent;
    bool responseReady; // When server write page.
    bool responseFinished; // When client read the page.
    condition_variable resolveResponse, resolveFinished;
} Request;

list<Request*> bufferRequests;
mutex canRequest, hasRequest, canCreate, canProcessing, canRead; // My mutexes to lock or unlock

condition_variable createdRequest, ableNewRequest; // Wake up server or Wake up a client. (in new request create step 0).

bool canWriteRequest;
int hasNewRequest;
int maxPages, clientsLength, lastPage=0;

void* server();
void* client(int id);
Request* make_request(int id);

int main(){
    // Allocating one thread for each line of matrix
    vector< thread > clients;

    cin >> maxPages;
    cin >> clientsLength;

    hasNewRequest = 0;
    canWriteRequest = true;

    thread s = thread(server);

    for(int i=0; i<clientsLength; i++){
	     clients.push_back( thread(client, i) );
    }



    for(int i=0; i<clientsLength; i++){
	     clients[i].join();
    }

    s.join();

    pthread_exit(NULL);
}

/***
  *
  * @function server
  * @description
  *    This function build pages and serve on buffer.
  *
***/
void* server(){

    while(lastPage < maxPages){
          // Waiting a client make a request.
          {
              unique_lock<mutex> lk(hasRequest);
              createdRequest.wait(lk, []{return hasNewRequest--;});
          }

          // In moment thread be locked waiting my response.
          Request *r = bufferRequests.front();// requests are processing in FIFO
          r->pageContent = ++lastPage;
          printf("[SERVER]: Creating new page %d to client %d\n", lastPage, r->idClient);
          r->responseReady = true;
          r->resolveResponse.notify_one();// WAKE UP THREAD because my response is ready

          // Locked waiting thread read response.
          unique_lock<mutex> read(canRead);
          r->resolveFinished.wait(read, [&]{return r->responseFinished;});

          // Clean response
          bufferRequests.pop_front();
          printf("[SERVER]: Removing page %d was read by client %d\n", r->pageContent, r->idClient);
          free(r); // free memory space.

          if(bufferRequests.size() < BUFFER_SIZE-1){
              canWriteRequest = true;
              ableNewRequest.notify_one(); // In case the a thread make one request and no have space..
          }else{
              canWriteRequest = false;
          }

          if(lastPage == maxPages){
              exit(0); // O Servidor encerra o processo.
          }
    }

    pthread_exit(NULL);
}

/***
  *
  * @function client
  * @description
  *    This function get pages stored at buffer.
  *
***/
void* client(int id){

   while(lastPage < maxPages){

          // Lock to add new Request on buffer
          while(bufferRequests.size() >= BUFFER_SIZE){// Impossible have 5 requests... is buffer size!
              unique_lock<mutex> lkcreate(canCreate);
              ableNewRequest.wait(lkcreate, []{return canWriteRequest;});
              hasNewRequest++;
          }

          Request* r = make_request(id);
          bufferRequests.push_back(r);

          printf("\t[CLIENT]: %d requesting new page\n", r->idClient);
          createdRequest.notify_one(); // Wake up SERVER to create a page!!!

          // Lock to wait server response.
          while(r->responseReady == 0){
              unique_lock<mutex> resp(canProcessing);
              r->resolveResponse.wait(resp, [&]{return r->responseReady;});
          }

          printf("\t[CLIENT]: %d reading page: %d\n", r->idClient, r->pageContent);
          //cout << "Cliente " << r->idClient << " recebeu a página " << r->pageContent << "\n";

          r->responseFinished = true;
          r->resolveFinished.notify_one(); // Wake up SERVER to clean space at buffer.

          if(lastPage == maxPages){
              pthread_exit(NULL);
          }
    }
    pthread_exit(NULL);
}

Request* make_request(int id){
    Request *r = (Request*) malloc(sizeof(Request));
    r->idClient = id;
    r->idBuffer = bufferRequests.size();
    r->pageContent = -1;
    r->responseReady = false;
    r->responseFinished = false;

    return r;
}