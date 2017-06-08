#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/*Let Ax = b be the square system of n linear equations.*/

/*Initial guess of the value of each x picked up based in nothing.*/
#define INITIAL_GUESS 1

/*Number of refinement-loops to be executed.*/
#define P 10

/*Row & column size of A and the column size of x and b*/
#define MATRIX_SIZE 2

/*Initializes the matrixes.*/
int a[MATRIX_SIZE][MATRIX_SIZE] = {0};
int x[MATRIX_SIZE] = {INITIAL_GUESS};
int b[MATRIX_SIZE] = {0};

/**/
int division_matrix[MATRIX_SIZE][MATRIX_SIZE] = {0};

/**/
int line_queue_size[MATRIX_SIZE];

/*Number of processors on the machine, thus, number of threads.*/
int threads_number = 0; //N

/*Variable that will guarantee that the loop counter will only 
incrmement one time per iteration, this is one of the critical regions.*/
int ignore = 0;

/*MUTEX for the access of the ignore variable.*/
pthread_mutex_t mutex_ignore = PTHREAD_MUTEX_INITIALIZER;

/*Counter for the number of refinements to be made, this is also a critical
region because all the threads will access it to increment its value and
to check if the required number of refinements were already made.*/
int k = 0;

/*MUTEX for the access of the k variable.*/
pthread_mutex_t mutex_k = PTHREAD_MUTEX_INITIALIZER;

/*Barrier to synchronize the threads after each refinement iteration.*/
pthread_barrier_t barrier;

void *jacobi_threaded(void *index);

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

    //Divides the calculus of all x variables between the threads
     if(MATRIX_SIZE == threads_number)
     {
        pthread_barrier_init(&barrier, NULL, threads_number);
         for (i = 0; i < threads_number; ++i) 
         {
             if(pthread_create(&thread[i], NULL, jacobi, (void*) i))
             {
                 printf("Something went wrong!\n");
                 exit(-1);
             }
         }
     }
     else if(MATRIX_SIZE < threads_number)
     {
        pthread_barrier_init(&barrier, NULL, MATRIX_SIZE);
         for (i = 0; i < MATRIX_SIZE; ++i) 
         {
             if(pthread_create(&thread[i], NULL, jacobi, (void*) i))
             {
                 printf("Something went wrong!\n");
                 exit(-1);
             }
         }   
     }
     else //(MATRIX_SIZE > threads_number)
     {
        pthread_barrier_init(&barrier, NULL, threads_number);
        int aux = MATRIX_SIZE;
        j = 0;
        for(i = 0; i < MATRIX_SIZE; ++i)
        {
            division_matrix[i % threads_number][j] = i;
            if( (i + 1) % threads_number == 0 )
            {
                line_queue_size[i] = j;
                j++;
            }
        }

        for (i = 0; i < threads_number; ++i) 
        {
            if(pthread_create(&thread[i], NULL, jacobi, (void*) &division_matrix[i]))
            {
                printf("Something went wrong!\n");
                exit(-1);
            }
        }
     }  
          
    /*Wait for all the threads to finish, so the main will only
    do it after them.*/
    pthread_exit(NULL);
    return 0;
}

void *jacobi_threaded(void *index) 
{
    /*Indexes.*/
    int i = 0;
    int j = 0;

    /*Variable to store the sum used in the Jacobi Method.*/
    int gama = 0;

    if(MATRIX_SIZE <= threads_number)
    {
        /*Saves the ID of the thread.*/
        int id_thread = (int) index;
    
        i = id_thread;

        while(k < P)
        {
            gama = 0;
            ignore = 1;
            for (j = 0; j < MATRIX_SIZE; ++j) 
            {
                if(j != i) 
                {
                    gama = gama + (a[i][j] * (x[j]));
                }
            }
            x[i] = (1/a[i][i]) * (b[i] - gama);
            pthread_barrier_wait(&barrier);
           
            /*Critical region that can only be accessed by one thread
            at each loop iteration>*/
            if(ignore == 0)
            {
                ignore = 1;
                k++;
            }
        }
    }
    else
    {
        /*Receives the division matrix*/
        int *variable_line = (int *) index;
    }

}