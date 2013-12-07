#include "RWLock_sem_librito.h"

using namespace std;

RWLock :: RWLock() {
	sem_init(&turnstile, 0, 1);
	sem_init(&roomEmpty, 0, 1);
	sem_init(&mutex,0,1);

	readers = 0;
}

RWLock :: ~RWLock() {
	sem_close(&mutex);
	sem_close(&turnstile);
	sem_close(&roomEmpty);
}

void RWLock :: rlock() {
	sem_wait(&turnstile);
	sem_post(&turnstile);
	sem_wait(&mutex);
		readers += 1;
		if (readers == 1){
			sem_wait(&roomEmpty); // first in locks
		}
	sem_post(&mutex);

// critical section for readers
	
}

void RWLock :: wlock() {
	sem_wait(&turnstile);
	sem_wait(&roomEmpty);

// critical section for writers

}

void RWLock :: runlock() {
	sem_wait(&mutex);
		readers -= 1;
		if (readers == 0){
			sem_post(&roomEmpty); // last out unlocks
		}
	sem_post(&mutex);
}

void RWLock :: wunlock() {
	sem_post(&turnstile);
	sem_post(&roomEmpty);	
}
