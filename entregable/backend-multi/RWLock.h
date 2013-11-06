#ifndef RWLock_h
#define RWLock_h
#include <iostream>
#include <semaphore.h>

class RWLock {
    public:
        RWLock();
        void rlock();
        void wlock();
        void runlock();
        void wunlock();

    private:
	sem_t rw;
	//~ sem_t algoMas;

	sem_t mutex_r;	
	sem_t mutex_w;	
	int readers;
	int writers;
};

#endif
