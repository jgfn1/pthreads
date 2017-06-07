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

typedef struct request {
    int idClient;
    int idBuffer; // to check if exists in max 5 spaces at buffer.
    int pageContent;
    bool responseReady; // When server write page.
    bool responseFinished; // When client read the page.
    condition_variable resolveResponse, resolveFinished;
} Request;

list<Request*> requests;
mutex canRequest, canCreate, canProcessing, canRead; // only for client requests...

condition_variable createdRequest, ableNewRequest; // Wake up server or Wake up a client. (in new request create step 0).

bool canWriteRequest, hasNewRequest;
int maxPages, clientsLength, lastPage=0;

void* server();
void* client(int id);
Request* make_request(int id);

int main(){
    // Allocating one thread for each line of matrix
    vector< thread > clients;
    cin >> maxPages;
    cin >> clientsLength;

    hasNewRequest = false;
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

    while(true){
          // Waiting a client make a request.
          {
              unique_lock<mutex> lk(canRequest);
              createdRequest.wait(lk, []{return hasNewRequest;});
              hasNewRequest = false;
          }

          // Lock threads to not make requests
          {
              if(requests.size() >= BUFFER_SIZE){
                  canWriteRequest = false;
                  lock_guard<mutex> lkcreate(canCreate);
              }
          }


          // Lock thread to wait my response.
          {
                lock_guard<mutex> resp(canProcessing);
          }
                Request *r = requests.front();// requests are processing in FIFO
                r->pageContent = ++lastPage;
                printf("[SERVER]: Creating new page %d to %d\n", lastPage, r->idClient);
                r->responseReady = true;
                r->resolveResponse.notify_all();// WAKE UP THREAD TO READ ..

                unique_lock<mutex> read(canRead);
                r->resolveFinished.wait(read, [&]{return r->responseFinished;});

                requests.pop_front();
                printf("[SERVER]: Removing page %d free space at buffer %d\n", r->pageContent, r->idBuffer);
                free(r); // free memory space.


          if(requests.size() < BUFFER_SIZE){
              canWriteRequest = true;
              ableNewRequest.notify_one(); // In case the a thread make one request and no have space..
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

   while(true){

          {
              lock_guard<mutex> lk(canRequest);
              hasNewRequest = true;
          }

          {
              while(requests.size() >= BUFFER_SIZE){// Impossible have 5 requests... is buffer size!
                  unique_lock<mutex> lkcreate(canCreate);
                  ableNewRequest.wait(lkcreate, []{return canWriteRequest;});
              }
          }

          Request* r = make_request(id);
          requests.push_back(r);
	        createdRequest.notify_one(); // Make server wake up, to create a page!!!

          printf("\t[CLIENT]: %d requesting new page at buffer[%d]\n", r->idClient,  r->idBuffer);

          while(r->responseReady == 0){
              unique_lock<mutex> resp(canProcessing);
              r->resolveResponse.wait(resp, [&]{return r->responseReady;});
          }

          lock_guard<mutex> iread(canRead);
          printf("\t[CLIENT]: %d reading page: %d at buffer[%d]\n", r->idClient, r->pageContent, r->idBuffer);
          r->responseFinished = true;
          r->resolveFinished.notify_all();

          if(lastPage == maxPages){
              pthread_exit(NULL);
          }
    }
    pthread_exit(NULL);
}

Request* make_request(int id){
    Request *r = (Request*) malloc(sizeof(Request));
    r->idClient = id;
    r->idBuffer = requests.size();
    r->pageContent = -1;
    r->responseReady = false;
    r->responseFinished = false;

    return r;
}