#define _XOPEN_SOURCE 600 //define para barreiras....
/*
		Thread Niggas
		Question 2 for Threads Project
		Created by nmf2
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define THREADS 4		// number of threads

char senha[11] = "ABCDEFGhIJ\0";	//senha a ser decifrada
char result[11] = { '\0' };			//resultado
int index_letra = 0;

pthread_t threads[THREADS];				//threads
pthread_barrier_t barrier;
pthread_mutex_t mutex_index_letra = PTHREAD_MUTEX_INITIALIZER;	//mutex para variável de index_letra

/*
	A barrier funciona da seguinte maneira: pthread_barrier_wait(barrier) deve ser chamado um número COUNT de vezes, especificado na inicialização da barreira,
	THREADS + 1 no nosso caso, (o +1 é por causa da main) quando todas as threads terminam e a main tbm, eu imprimo o resultado. Sincronizando todas as threads
	inclusive a da main.
*/


int incrementar_index_letra(int id){	//função para incrementar o index_letra 
	int i;
	pthread_mutex_lock(&mutex_index_letra);	//lock mutex do index_letra...
	printf("\tindex_letra atual: %d, chamado pela thread: %d \n", index_letra, id);
	if(index_letra < 11){		//se o index_letra global do mutex for menor q 11
		i = index_letra;		//faça o index_letra atual ser igual ao global
		index_letra++;		//incremente o global...
	}
	pthread_mutex_unlock(&mutex_index_letra);	//unlock mutex do index_letra
	return i;
}


void* decifra(void * arg){
	int i = ((int) arg)%100;	//index_letra do caractere de senha[] a ser decifrado, início da thread
	int id = ((int) arg)/100;	//id dessa thread
	int teste = 32;	//32 em ascii é (espaço), antes são coisas como ESC, backspace, etc. Depois são letras, símbolos e números...
	
	while(index_letra < 11){ 			//enquanto o index_letra global for menor que 11, a thread ainda irá existir e trabalhar
		printf("thread %d trabalhando com index %d.\n", id, i);
		
		for(teste = 32; teste != senha[i]; teste++); //teste começa com 32, enquanto for diferente do caractere a ser descifrado, incrementa-se teste
		printf("fim: teste= '%c'\n", teste);
		result[i] = teste;	//salvar letra decifrada
		i = incrementar_index_letra(id);	//incrementa index_letra seguramente...
	}
	printf("thread %d na barreira...\n", id);
	pthread_barrier_wait(&barrier);
	printf("thread %d finalizada.\n", id);
	pthread_exit(NULL);		//index_letra >= 11, essa thread não é mais necessária
}
int main (){
	pthread_barrier_init(&barrier, NULL, THREADS + 1);

	for(int id = 0; id < THREADS; id++){
		pthread_mutex_lock(&mutex_index_letra);	//lock mutex do index_letra...

		pthread_create(&threads[id], NULL, decifra, (void *) index_letra + 100*id); 	//cria thread com index_letra atual... seja ele qual for
		
		printf("\tindex_letra atual: %d, chamado pela main\n", index_letra);
		if(index_letra < 11)		//se o index_letra global do mutex for menor q 11
			index_letra++;		//incremente o global...
		
		pthread_mutex_unlock(&mutex_index_letra);	//unlock mutex do index_letra					//incrementa index_letra..
		/* dado = (void *) index_letra + 100*id isso aqui serve para que eu possa mandar o index_letra inicial da thread e o seu id no mesmo número, 
			id = dado/100, index_letra = dado%100;
			Sim, isso é preguiça de criar uma struct... qual é? Vou ser um engenheiro, tenho que ser engenhoso!
		*/
	}

	pthread_barrier_wait(&barrier);
	pthread_barrier_destroy(&barrier);
	printf("\n\t\t ### Senha decifrada: \"%s\", senha dada: \"%s\" ###\n\n", result, senha);
	pthread_exit(NULL);
}