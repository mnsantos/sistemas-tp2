#ifndef RWLock_h
#define RWLock_h
#include <iostream>
#include <semaphore.h>
#include <assert.h>

class RWLock {
    public:
        RWLock();
        ~RWLock();
        void rlock();
        void wlock();
        void runlock();
        void wunlock();

    private:
		sem_t turnstile;
		sem_t roomEmpty;
		sem_t mutex;	
		int readers;
};

#endif
