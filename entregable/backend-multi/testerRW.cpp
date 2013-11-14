#include <iostream>
#include "RWLock_sem.h"
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
	return NULL;
}

void *escritor(void * p_miNumero){
	int miNumero = *((int *) p_miNumero);
	lock.wlock();
	variable_a_leer_escribir++;
	
	// Esperar un valor aleatorio de ms.
	usleep(2000); //4ms
	//
	
	cout<<"Escribi: "<<variable_a_leer_escribir<<" y soy: "<<miNumero<<endl;
	lock.wunlock();
	return NULL;
}


int main(int argc, char **argv)
{
	int cant_lectores = 5;
	int cant_escritores = 5;
	
	pthread_mutex_init(&nomepiso, NULL);
	
	pthread_t threadR[cant_lectores]; int tidsR[cant_lectores];
	int tid;
	for (tid = 0; tid < cant_lectores; ++tid){
		tidsR[tid] = tid;
		pthread_create(&threadR[tid], NULL, lector ,&tidsR[tid]);
	}


	pthread_t threadW[cant_escritores];
	int tidsW[cant_lectores];
	for (tid = 0; tid < cant_escritores; ++tid){
		tidsW[tid] = tid;
		pthread_create(&threadW[tid], NULL, escritor , &tidsW[tid]);
	}
	
	//pthread exit(status)
	int * status;
	
	for (tid = 0; tid < cant_lectores; ++tid){
		pthread_join(threadR[tid], NULL);
	}


	for (tid = 0; tid < cant_lectores; ++tid){
		pthread_join(threadW[tid], NULL);
	}

	//cout<<"termine"<<endl;
	
	return 0;
	
}
