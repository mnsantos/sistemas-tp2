#ifndef RWLock_h
#define RWLock_h
#include <iostream>
#include <pthread.h>

class RWLock {
    public:
        RWLock();
        ~RWLock();
        void rlock();
        void wlock();
        void runlock();
        void wunlock();

    private:
		pthread_mutex_t rw_lock;
		
		pthread_mutex_t accessR;
		pthread_cond_t no_writers;
		
		pthread_mutex_t accessW;
		pthread_cond_t no_readers;
		
		pthread_mutex_t mutex_r;	
		int readers;

		pthread_mutex_t mutex_w;	
		int writers;
};

#endif


