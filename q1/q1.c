/*
		Thread Niggas
		Question 1 for Threads Project
		Created by nmf2
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#define N 10			// number of files
#define P 10			//tipos de produtos

int total_produtos = 0;
pthread_mutex_t index_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t produtos_mutex[N] = {PTHREAD_MUTEX_INITIALIZER};
FILE *files[N];			

int produtos[P+1] = { 0 };

void *treatFile(void *arg){
	int i = (int) arg;
	char fileName[7] = {'\0'};
	char problem[4] = {'\0'};							//problem type
	sprintf(fileName, "%d.in", i+1); 			//print file name in fileName
		
	files[i] = fopen(fileName, "r");			//open file
	{//file opened
		while(fgets(problem, 4, files[i]), !feof(files[i])){ //
			int prob_num = atoi(problem);			//convert string to number...
			printf("thread: %d, atoi: %d\n", i, prob_num);
			pthread_mutex_lock(&produtos_mutex[prob_num]);	
			produtos[prob_num]++;							
			pthread_mutex_unlock(&produtos_mutex[prob_num]);	
			total_produtos++;
		}
	}//file to be closed
	fclose(files[i]);
	pthread_exit(NULL);
}

int main (){
	pthread_t thread[N];
	int i = 0;

	for (; i < N; i++){
		if(pthread_create(&thread[i], NULL, treatFile, (void*) i)){
			printf("Something wrong\n");
			exit(-1);
		}
	}

	for (i = 1; i < P+1; i++){
		pthread_join(thread[i-1], NULL);
		//printf("Produtos do tipo %d:\n\tQtde: %d, %%%f\n", i, produtos[i], ((float) produtos[i])/total_produtos);
	}
	for (i = 1; i < P+1; i++){
		//pthread_join(thread[i-1], NULL);
		printf("Produtos do tipo %d:\n\t\tQtde: %d, %%%.2f\n", i, produtos[i], 100*((float) produtos[i])/total_produtos);
	}
	printf("Total de produtos: %d\n", total_produtos);

	pthread_exit(NULL);
}