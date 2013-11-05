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
		sem_t read_lock;
		//~ sem_t read_lock;
		int readers;
		int writers;
};

#endif
