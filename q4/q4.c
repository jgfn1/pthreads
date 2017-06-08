#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/*Let Ax = b be the square system of n linear equations.*/

/*Initial guess of the value of each x picked up based in nothing.*/
#define INITIAL_GUESS 1

/*Number of refinement-loops to be executed.*/
#define P 10

/*Row & column size of A and the column size of x and b*/
#define MATRIX_SIZE 3

/*Initializes the matrixes.*/
int a[MATRIX_SIZE][MATRIX_SIZE] = {0};
int x[MATRIX_SIZE] = {INITIAL_GUESS};
int b[MATRIX_SIZE] = {0};

/*Number of processors on the machine, thus, number of threads.*/
int threads_number = 0; //N

/*Index of the next x available to be gotten by a thread, in other words,
this is one of the critical regions.*/
int x_available = 0;

/*MUTEX for the access of the x_available variable.*/
pthread_mutex_t mutex_x_available = PTHREAD_MUTEX_INITIALIZER;

/*Counter for the number of refinements to be made, this is also a critical
region because all the threads will access it to increment its value and
to check if the required number of refinements were already made.*/
int k = 0;

/*MUTEX for the access of the k variable.*/
pthread_mutex_t mutex_k = PTHREAD_MUTEX_INITIALIZER;

void *jacobi_thread(void *index);

int main() 
{
    int i = 0;
    int j = 0;
    printf("Insert the number of threads to be created: \n");
    scanf("%d", &threads_number);
    pthread_t *thread = (pthread_t*) malloc(threads_number * sizeof(pthread_t));
    
    /*Assigns values to the matrix in a way that it will be diagonally 
    dominant (minimum requirement of the Jacobi Method).*/
    for (i = 0; i < MATRIX_SIZE; ++i)
    {
        b[i] = i + 1;
        for(j = 0 ; j < MATRIX_SIZE; ++j)
        {
            if(i == j)
                a[i][j] = (3 * (i + j)) + MATRIX_SIZE; 
            else 
                a[i][j] = i + j;
        }
    }

    /*Creates all the threads.*/
    for (i = 0; i < threads_number; ++i) 
    {
        if(pthread_create(&thread[i], NULL, jacobi_thread, (void*) i)){
            printf("Something went wrong!\n");
            exit(-1);
        }
    }

    /*Wait for all the threads to finish, so the main will only
    do it after them.*/
    pthread_exit(NULL);
    return 0;
}

void *jacobi_thread(void *index) 
{
    /*Saves the ID of the thread.*/
    int id_thread = (int) index;
    
    /*Indexes.*/
    int i = 0;
    int j = 0;

    /*Variable to store the sum used in the Jacobi Method.*/
    int gama = 0;

    /*Accesses the critical region of the available x's.*/
    pthread_mutex_lock(&mutex_x_available);
    while(x_available < MATRIX_SIZE)
    {
        i = x_available;
        x_available++;
        pthread_mutex_unlock(&mutex_x_available);

        gama = 0;
        for (j = 0; j < MATRIX_SIZE; ++j) 
        {
            if(j != i) 
            {
                gama = gama + (a[i][j] * (x[j]));
            }
        }
        x[i] = (1/a[i][i]) * (b[i] - gama);

        pthread_mutex_lock(&mutex_x_available);    
    }
    pthread_mutex_unlock(&mutex_x_available);


    while(k < P)
    {
        k++; //REGIAO CRITICA
        for (i = 0; i < MATRIX_SIZE; ++i) //For deve ser alterado, REGIAO CRITICA
        {
            gama = 0;
            for (j = 0; j < MATRIX_SIZE; ++j) 
            {
                if(j != i) 
                {
                    gama = gama + (a[i][j] * (x[j]));
                }
            }
            x[i] = (1/a[i][i]) * (b[i] - gama);
        }

    }
}