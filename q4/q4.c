#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define INITIAL_GUESS 1
#define P 10
#define MATRIX_SIZE 3

int a[N][N] = {0};
int x[N] = {INITIAL_GUESS};
int b[N] = {0};

//Number of processors on the machine, thus, number of threads.
int threads_number; //N

void *jacobi(void *index);

int main() 
{
    int i;
    int j;
    printf("Insert the number of threads to be created: \n");
    scanf("%d", &threads_number);
    pthread_t *thread = (pthread_t*) malloc(threads_number * sizeof(pthread_t));
    
    // printf("\n");
    for (i = 0; i < MATRIX_SIZE; ++i)
    {
        b[i] = i + 1;
        for(j = 0 ; j < MATRIX_SIZE; ++j)
        {
            if(i == j)
                /*Assigning a[i][j] likes this so the matrix will be diagonally 
                dominant (minimum requirement of the Jacobi Method)*/
                a[i][j] = (3 * (i + j)) + MATRIX_SIZE; 
            else 
                a[i][j] = i + j;
            // printf("%d ", a[i][j]);
        }
        // printf("\n");
    }
    
    /*printf("\n");
    for (i = 0; i < N; ++i)
    {
        printf("%d ", b[i]); //Testing the building of array 'b'
    }
    printf("\n");*/

    //Divides the calculus of all x variables between the threads
    if(MATRIX_SIZE == threads_number)
    {
        for (i = 0; i < threads_number; ++i) 
        {
            if(pthread_create(&thread[i], NULL, jacobi, (void*) i)){
                printf("Something went wrong!\n");
                exit(-1);
            }
        }
    }
    else if(MATRIX_SIZE < threads_number)
    {
        for (i = 0; i < MATRIX_SIZE; ++i) 
        {
            if(pthread_create(&thread[i], NULL, jacobi, (void*) i)){
                printf("Something went wrong!\n");
                exit(-1);
            }
        }   
    }
    else //(MATRIX_SIZE > threads_number)



    pthread_exit(NULL);
    return 0;
}

void *jacobi(void *index) 
{
    int index1 = (int) index;
    // printf("Thread:: %d\n", index1);
    int k = 0;
    int i;
    int j;
    int gama;
    while(k < P)
    {
        for (i = 0; i < N; ++i) 
        {
            gama = 0;
            for (j = 0; j < N; ++j) 
            {
                if(j != i) 
                {
                    gama = gama + (a[i][j] * (x[j]));
                }
            }
            x[i] = (1/a[i][i]) * (b[i] - gama);
        }
        k++;

    }
}