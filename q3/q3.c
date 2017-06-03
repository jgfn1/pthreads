#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFFER_SIZE 5
#define N_THREADS 5

//int counterUsedWithMutex=0, counterUsedWithoutMutex=0;
int buffer[5];
int bufferNitens = 0;
int indexPageProducer = 0;
int indexPageConsumer = 0;

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

/***
  *
  * @function servidor
  * @description
  *    This function build pages and serve on buffer.
  *
***/
void* servidor(){
      int i;
      pthread_mutex_lock(&mutex);
      while(bufferNitens > 0) pthread_cond_wait(&bufferEmpty, &mutex);
      for(i=0; i<BUFFER_SIZE; i++) buffer[i] = indexPageProducer++; // Seta o número de página que está a ser produzida

      bufferNitens = BUFFER_SIZE;  // Buffer não está vazio!

      //printf("Servidor vai fazer %d páginas: \n", BUFFER_SIZE);
      for(i=0; i<BUFFER_SIZE; i++) printf("\t%d\n", buffer[i]);

      printf("Páginas produzidas:  %d\n", bufferNitens);

      if(bufferNitens == BUFFER_SIZE){// Irá disparar o buffer cheio, e liberar o mutex
          pthread_cond_broadcast(&bufferFill);
          pthread_mutex_unlock(&mutex);
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
      int* id = (int*) threadid;
      pthread_mutex_lock(&mutex);
      while(bufferNitens == 0) pthread_cond_wait(&bufferFill, &mutex);

      printf("CLIENTE %d recebeu a página %d\n", *id, buffer[indexPageConsumer % 5]);
      bufferNitens--;
      indexPageConsumer++;

      if(bufferNitens == 0){
          pthread_cond_signal(&bufferEmpty);
      }
      pthread_mutex_unlock(&mutex);
      pthread_exit(NULL);
}

int main (int argc, char *argv[]){
    int i, j, id[N_THREADS];
    pthread_t server, c[N_THREADS];

    for(j=0; j<3; j++){

        pthread_create(&server, NULL, servidor, NULL);
        for (i=0; i < N_THREADS; i++) {
            id[i] = i;
            pthread_create(&c[i], NULL, cliente, (void*) &id[i]);
        }
        pthread_join(server, NULL);
        for (i=0; i < N_THREADS; i++) {
            pthread_join(c[i], NULL);
        }
    }

    pthread_exit(NULL);
}