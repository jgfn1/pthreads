#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define initial_guess 1
#define P 10
#define n 3

int** A;
int* X;
int* B;

//Number of processors on the machine, thus, number of threads.
int threads_number; //N

void *jacobi(void *index);

int main() 
{
    printf("Insert the number of threads to be created: \n");
    scanf("%d", &threads_number);
    pthread_t *thread = (pthread_t*) malloc(threads_number * sizeof(pthread_t));
    
    int i;
    for (i = 0; i < threads_number; ++i) 
    {
        if(pthread_create(&thread[i], NULL, jacobi, (void*) i)){
            printf("Something wrong\n");
            exit(-1);
        }
    }

    pthread_exit(NULL);
    return 0;
}

void *jacobi(void *index) 
{
    int index1 = (int) index;
    int k = 0;
    while(k < P)
    {
        for (int i = 0; i < n; ++i) 
        {
            int gama = 0;
            for (int j = 0; j < n; ++j) 
            {
                if(j != i) 
                {
                    gama = gama + (A[i][j] * (X[j]));
                }
            }
            X[i] = (1/A[i][i]) * (B[i] - gama);
        }
        k++;
    }
    return NULL;
}