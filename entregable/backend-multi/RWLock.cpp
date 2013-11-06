#include "RWLock.h"

RWLock :: RWLock() {
	sem_init(&rw, 0, 1);

	sem_init(&mutex_r,0,1);
	sem_init(&mutex_w,0,1);
	readers = 0;
	writers = 0;
}

void RWLock :: rlock() {
	
	sem_wait(&mutex_w);
		if (writers==0){
			sem_signal(&mutex_w);
			sem_wait(&mutex_r);
				readers++;
				if (readers == 1){
					sem_wait(&rw);
			sem_signal(&mutex_r);
			//Critica
		}
		else sem_signal(&mutex_w);
			
}

void RWLock :: wlock() {

}

void RWLock :: runlock() {
 
}

void RWLock :: wunlock() {

}
