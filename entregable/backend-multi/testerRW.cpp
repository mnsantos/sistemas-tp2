#include <iostream>
#include <stdlib.h>
//~ #include "RWLock_sem.h"
#include "RWLock_sem_librito.h"
#include <unistd.h>
#include <pthread.h>

using namespace std;

//Variables globales
RWLock lock;
pthread_mutex_t nomepiso;

int variable_a_leer_escribir = 0;

void *lector(void * p_miNumero){
	int miNumero = *((int *) p_miNumero);
	lock.rlock();
	
	// Esperar un valor aleatorio de ms.
	usleep(2000); //4ms
	//
	
	pthread_mutex_lock(&nomepiso);
	cout << "Lei: "<<variable_a_leer_escribir<<" y soy: "<<miNumero<<endl;
	pthread_mutex_unlock(&nomepiso);
	
	lock.runlock();
	pthread_mutex_lock(&nomepiso);
	cout << "termine: " << miNumero << endl;
	pthread_mutex_unlock(&nomepiso);
	return NULL;
}

void *escritor(void * p_miNumero){
	int miNumero = *((int *) p_miNumero);
	lock.wlock();
	variable_a_leer_escribir++;
	
	// Esperar un valor aleatorio de ms.
	usleep(2000); //4ms
	//
	pthread_mutex_lock(&nomepiso);
	cout<<"Escribi: "<<variable_a_leer_escribir<<" y soy: "<<miNumero<<endl;
	pthread_mutex_unlock(&nomepiso);
	lock.wunlock();
	pthread_mutex_lock(&nomepiso);
	cout << "termine: " << miNumero << endl;
	pthread_mutex_unlock(&nomepiso);
	return NULL;
}


int main(int argc, char **argv)
{
	if(argc != 3){
		cout << "faltan parametros, se esperaba ./test cant_lectores cant_escritores" << endl;
		return 0;
	}
	int cant_lectores = atoi(argv[1]);
	int cant_escritores = atoi(argv[2]);
	
	cout << "Lectores: " << cant_lectores << endl;
	cout << "Escritores: " << cant_escritores << endl;
	
	pthread_mutex_init(&nomepiso, NULL);
	int tid;
	
	pthread_t threadW[cant_escritores];
	int tidsW[cant_escritores];
	
	for (tid = 0; tid < cant_escritores; ++tid){
		tidsW[tid] = tid;
		pthread_create(&threadW[tid], NULL, escritor , &tidsW[tid]);
	}
	
	
	pthread_t threadR[cant_lectores]; int tidsR[cant_lectores];

	for (tid = 0; tid < cant_lectores; ++tid){
		tidsR[tid] = tid;
		pthread_create(&threadR[tid], NULL, lector ,&tidsR[tid]);
	}


	
	//pthread exit(status)
	//~ int * status;
	
	for (tid = 0; tid < cant_lectores; ++tid){
		pthread_join(threadR[tid], NULL);
	}


	for (tid = 0; tid < cant_escritores; ++tid){
		pthread_join(threadW[tid], NULL);
	}
	
	cout << endl << endl <<"termine 1er caso"<<endl;
	
	for (tid = 0; tid < cant_lectores; ++tid){
		tidsR[tid] = tid;
		pthread_create(&threadR[tid], NULL, lector ,&tidsR[tid]);
	}	
	
	for (tid = 0; tid < cant_escritores; ++tid){
		tidsW[tid] = tid;
		pthread_create(&threadW[tid], NULL, escritor , &tidsW[tid]);
	}

	//pthread exit(status)
	//~ int * status;
	
	for (tid = 0; tid < cant_lectores; ++tid){
		pthread_join(threadR[tid], NULL);
	}


	for (tid = 0; tid < cant_escritores; ++tid){
		pthread_join(threadW[tid], NULL);
	}
	
	cout << endl << endl <<"termine 2 caso"<<endl;
	
		for (tid = 0; tid < cant_lectores/2; ++tid){
		tidsR[tid] = tid;
		pthread_create(&threadR[tid], NULL, lector ,&tidsR[tid]);
	}	
	
	for (tid = 0; tid < cant_escritores/2; ++tid){
		tidsW[tid] = tid;
		pthread_create(&threadW[tid], NULL, escritor , &tidsW[tid]);
	}
	
	for (tid = cant_lectores/2; tid < cant_lectores; ++tid){
		tidsR[tid] = tid;
		pthread_create(&threadR[tid], NULL, lector ,&tidsR[tid]);
	}	
	
	for (tid = cant_escritores/2; tid < cant_escritores; ++tid){
		tidsW[tid] = tid;
		pthread_create(&threadW[tid], NULL, escritor , &tidsW[tid]);
	}


	//pthread exit(status)
	//~ int * status;
	
	for (tid = 0; tid < cant_lectores; ++tid){
		pthread_join(threadR[tid], NULL);
	}


	for (tid = 0; tid < cant_escritores; ++tid){
		pthread_join(threadW[tid], NULL);
	}
	
	cout << endl << endl <<"termine 3er caso"<<endl;
	
	return 0;
	
}
