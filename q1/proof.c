/*
		Thread Niggas
		Proof that q1.c works, this file does the same thing except it doesn't use threads
		Created by nmf2
		
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define N 11			// number of files
#define P 11			//tipos de produtos

 
int tipos_produtos = 0, total_produtos = 0;				//quantidade de tipos de produtos diferentes existentes.
FILE *files[N];			

int produtos[P] = { 0 };			

void treatFile(int arg){
	int i = (int) arg;
	char file_name[7] = {'\0'};
	char problem[4] = {'\0'};							//problem type
	sprintf(file_name, "%d.in", i); 			//print file name in file_name
		
	files[i] = fopen(file_name, "r");			//open file
	{//file opened
		while(fgets(problem, 4, files[i]), !feof(files[i])){ //
			int prob_num = atoi(problem);			//convert string to number...
			printf("file: %d, atoi: %d\n", i, prob_num);
			produtos[prob_num]++;							
			total_produtos++;
		}

	}//file to be closed
	fclose(files[i]);
}

int main (){
	int i = 1;
 	
	for (; i < N; i++){
		treatFile(i);
	}

	for (i = 1; i < P; i++){
		printf("Produtos do tipo %d:\n\t\tQtde: %d, %%%f\n", i, produtos[i], ((float) produtos[i])*(100)/total_produtos);
	}
	printf("Total de produtos: %d\n", total_produtos);

	return 0;
}
