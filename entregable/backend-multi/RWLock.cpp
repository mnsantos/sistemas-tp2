#include "RWLock.h"

RWLock :: RWLock() {
	pthread_mutex_init(&rw, NULL);
	pthread_mutex_init(&accessR, NULL);
	pthread_mutex_init(&accessW, NULL);
	pthread_mutex_init(&mutex_r, NULL);
	pthread_mutex_init(&mutex_w, NULL);
	pthread_cond_init (&no_readers, NULL);
	pthread_cond_init (&no_writers, NULL);

	readers = 0;
	writers = 0;
	
	//~ sem_init(&rw, 0, 1);
	//~ sem_init(&accessR, 0, 0);	//el primero espera.
	//~ sem_init(&accessW, 0, 0);
	//~ sem_init(&mutex_r,0,1);
	//~ sem_init(&mutex_w,0,1);
}

RWLock::~RWLock(){
	pthread_mutex_destroy(&rw);
	pthread_mutex_destroy(&accessR);
	pthread_mutex_destroy(&accessW);
	pthread_mutex_destroy(&mutex_r);
	pthread_mutex_destroy(&mutex_w);
	pthread_cond_destroy(&no_readers);
	pthread_cond_destroy(&no_writers);
}

void RWLock :: rlock() {
	
	//~ sem_wait(&mutex_r);				//pedimos mutex_r para evitar deadlock.
	pthread_mutex_lock(&mutex_r);
	//~ sem_wait(&mutex_w);				//vemos si hay escritores escribiendo/esperando
	pthread_mutex_lock(&mutex_w);

	if (writers == 0){				//si no hay escritores => ESTADO LECTURA
		
									//vemos si hay otros lectores.
		if (readers == 0){			//si soy el primero
			pthread_mutex_lock(&rw_lock);
			//~ sem_wait(&rw_lock);		//tomo el rw_lock
		}
		readers++;					//sino me sumo al resto.
		
		pthread_mutex_unlock(&mutex_r);
		pthread_mutex_unlock(&mutex_w);
		//~ sem_signal(&mutex_r);		//cedemos los mutex.
		//~ sem_signal(&mutex_w);
		
		///lectura.
		
	}else{							//hay escritores => ESTADO ESCRITURA
				
		readers++;					//nos unimos a los lectores en espera.
		//~ sem_signal(&mutex_r);		
		//~ sem_signal(&mutex_w);		//importante pedir el mutex_r antes de ceder mutex_w
		pthread_mutex_unlock(&mutex_r);
		pthread_mutex_unlock(&mutex_w);
		//~ sem_wait(&accessR);			//esperamos el cambio de estado.
		pthread_mutex_lock(&accessR);
		pthread_cond_wait(&no_writers,&accessR);
		pthread_mutex_unlock(&accessR); 
									//cuando ganamos el acceso, writers deberia ser cero.
		pthread_mutex_lock(&mutex_r);
		//~ sem_wait(&mutex_r);
		if (readers == 0){			//si soy el primero
			//~ sem_wait(&rw_lock);		//tomo el rw_lock
			pthread_mutex_lock(&rw_lock);
		}
		readers++;					//hacemos presencia		
		//~ sem_signal(&mutex_r);
		pthread_mutex_unlock(&mutex_r);
		
		///lectura.
}

void RWLock :: wlock() {
	//~ sem_wait(&mutex_r);				//vemos si hay lectores leyendo/esperando
	pthread_mutex_lock(&mutex_r);
	//~ sem_wait(&mutex_w);
	pthread_mutex_lock(&mutex_w);
	writers++;					
	//~ sem_signal(&mutex_w);
	pthread_mutex_unlock(&mutex_w);
	if(readers == 0){				//si no hay
					
		
		//~ sem_signal(&mutex_r);
		pthread_mutex_unlock(&mutex_r);
		//~ sem_wait(&rw_lock); 		//esperamos nuestro turno, no hay escritura concurrente todos juntos, este semaforo deberia funcionar.		
		pthread_mutex_lock(&rw_lock);
		
		///escritura.
		
	}else{


									//si hay lectores

		//~ sem_signal(&mutex_r);
		pthread_mutex_unlock(&mutex_r);
		//~ sem_wait(&accessW);			//pedimos acceso.
		pthread_mutex_lock(&accessW);
		pthread_cond_wait(&no_readers,&accessW);
		pthread_mutex_unlock(&accessW);
									//con el acceso, no deberia haber lectores.
		//~ sem_wait(&mutex_w);
		pthread_mutex_lock(&mutex_w);
		writers++;
		//~ sem_signal(&mutex_w);
		pthread_mutex_unlock(&mutex_w);
		//~ sem_wait(&rw_lock);		//tomo el rw_lock
		pthread_mutex_lock(&rw_lock);
		
		///escritura.
	}
}

void RWLock :: runlock() {
	//~ sem_wait(&mutex_r);
	pthread_mutex_lock(&mutex_r);
	//~ sem_wait(&mutex_w);
	pthread_mutex_lock(&mutex_w);
	readers--;						//salimos
	if (readers == 0){				//si somos el ultimo en salir
									//vemos si hay escritores esperando
		for(int i = 0; i < writers; i++){
			//~ sem_signal(&accessW);			//si hay escritores, les cedemos el paso.
			pthread_mutex_lock(&accessW);
			pthread_cond_broadcast(&no_readers);
			pthread_mutex_unlock(&accessW);
		}
		writers = 0;				//seteamos en cero escritores.

		//~ sem_signal(&rw_lock);		//cedemos el lock.
		pthread_mutex_unlock(&rw_lock);
	}
	//~ sem_signal(&mutex_w);
	//~ sem_signal(&mutex_r);
	pthread_mutex_unlock(&mutex_w);
	pthread_mutex_unlock(&mutex_r);
		
}

void RWLock :: wunlock() {
	//~ sem_wait(&mutex_r);
	pthread_mutex_lock(&mutex_r);
	//~ sem_wait(&mutex_w);
	pthread_mutex_lock(&mutex_w);
	writers--;
	if (writers == 0){
		for (int i = 0; i < readers; i++){
			//~ sem_signal(&accessR);
			pthread_mutex_lock(&accessR);
			pthread_cond_broadcast(&no_writers);
			pthread_mutex_unlock(&accessR);
		}
		readers = 0;
	}
	//~ sem_signal(&rw_lock);
	//~ sem_signal(&mutex_w);
	//~ sem_signal(&mutex_r);
	pthread_mutex_unlock(&rw_lock);
	pthread_mutex_unlock(&mutex_w);
	pthread_mutex_unlock(&mutex_r);
}
