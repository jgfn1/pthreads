#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFFER_SIZE 5

//int counterUsedWithMutex=0, counterUsedWithoutMutex=0;
int buffer[5];
int bufferNitens = 0;
int maxPages, indexPageProducer = 0, indexPageConsumer = 0;

/***
    Quando o buffer estiver vazio o servidor vai encher o buffer.

    Os clientes basicamente tentam ler do buffer, (o indice de leitura é um mutex)

***/

/***
    Na verdade tudo que o servidor faz é limpar um determinado espaço de buffer, ou seja coloca-lo para zero.

    O Cliente preenche o espaço de buffer com um numero e diz ao servidor: pode vir limpar...
      O Numero que o cliente colocou é a página que ele quer requisitar.

*/

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t bufferEmpty = PTHREAD_COND_INITIALIZER;
pthread_cond_t bufferFill = PTHREAD_COND_INITIALIZER;

void* cliente(void *threadid);
void* servidor();

int main (int argc, char *argv[]){
    pthread_t server;
    int i, j, qtdClientes, qtdExecucoes;

    scanf("%d", &qtdClientes);
    scanf("%d", &maxPages);

    pthread_t* clientes = (pthread_t*) malloc(sizeof(pthread_t) * qtdClientes);
    int* id = (int*) malloc(sizeof(int) * qtdClientes);

    //for(j=0; j<qtdExecucoes; j++){

        pthread_create(&server, NULL, servidor, NULL);
        for (i=0; i < qtdClientes; i++) {
            id[i] = i;
            pthread_create(&clientes[i], NULL, cliente, (void*) &id[i]);
        }
      /*  pthread_join(server, NULL);
        for (i=0; i <qtdClientes; i++) {
            pthread_join(clientes[i], NULL);
        }
    }*/

    pthread_exit(NULL);
}


/***
  *
  * @function servidor
  * @description
  *    This function build pages and serve on buffer.
  *
***/
void* servidor(){
      while(indexPageProducer < maxPages){
          int i;
          pthread_mutex_lock(&mutex);
          while(bufferNitens > 0) pthread_cond_wait(&bufferEmpty, &mutex);
          for(i=0; i<BUFFER_SIZE; i++) buffer[i] = indexPageProducer++; // Seta o número de página que está a ser produzida

          bufferNitens = BUFFER_SIZE;  // Buffer não está vazio!

          if(bufferNitens == BUFFER_SIZE){// Irá disparar o buffer cheio, e liberar o mutex
              pthread_cond_broadcast(&bufferFill);
              pthread_mutex_unlock(&mutex);
          }
      }
}

/***
  *
  * @function cliente
  * @description
  *    This function get pages stored at buffer.
  *
***/
void* cliente(void *threadid){
      while(indexPageConsumer < maxPages){
          int* id = (int*) threadid;
          pthread_mutex_lock(&mutex);
          while(bufferNitens == 0) pthread_cond_wait(&bufferFill, &mutex);

          printf("CLIENTE %d recebeu a página %d\n", *id, buffer[indexPageConsumer % BUFFER_SIZE]);
          bufferNitens--;
          indexPageConsumer++;

          if(bufferNitens == 0){
              pthread_cond_signal(&bufferEmpty);
          }else{
              pthread_cond_signal(&bufferFill); // To wake another client..
          }
          pthread_mutex_unlock(&mutex);
      }
}