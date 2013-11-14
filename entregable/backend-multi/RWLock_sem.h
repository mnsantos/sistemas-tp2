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
		sem_t rw_lock;
		sem_t accessR;
		sem_t accessW;
		
		sem_t mutex;	
		int readers;
		int readers_waiting;
	
		int writers;
		int writers_waiting;
};

#endif
