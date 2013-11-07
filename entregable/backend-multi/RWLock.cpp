#include "RWLock.h"

RWLock :: RWLock() {
	sem_init(&rw, 0, 1);
	sem_init(&accessR, 0, 0);	//el primero espera.
	sem_init(&accessW, 0, 0);
	
	sem_init(&mutex_r,0,1);
	sem_init(&mutex_w,0,1);
	readers = 0;
	writers = 0;
}

void RWLock :: rlock() {
	
	sem_wait(&mutex_r);				//pedimos mutex_r para evitar deadlock.
	sem_wait(&mutex_w);				//vemos si hay escritores escribiendo/esperando
	if (writers == 0){				//si no hay escritores => ESTADO LECTURA
		
		//~ sem_signal(&mutex_w);
		//~ sem_wait(&mutex_r);			//vemos si hay otros lectores.
		if (readers == 0){			//si soy el primero
			sem_wait(&rw_lock);		//tomo el rw_lock
		}
		readers++;					//sino me sumo al resto.
		sem_signal(&mutex_r);		//cedemos los mutex.
		sem_signal(&mutex_w);
		
		///lectura.
		
	}else{							//hay escritores => ESTADO ESCRITURA
		//~ sem_wait(&mutex_r);			
		readers++;					//nos unimos a los lectores en espera.
		sem_signal(&mutex_r);		
		sem_signal(&mutex_w);		//importante pedir el mutex_r antes de ceder mutex_w
		sem_wait(&accessR);			//esperamos el cambio de estado.
									//cuando ganamos el acceso, writers deberia ser cero.
		sem_wait(&mutex_r);
		if (readers == 0){			//si soy el primero
			sem_wait(&rw_lock);		//tomo el rw_lock
		}
		readers++;					//hacemos presencia		
		sem_signal(&mutex_r);
		
		///lectura.
}

void RWLock :: wlock() {
	sem_wait(&mutex_r);				//vemos si hay lectores leyendo/esperando
	sem_wait(&mutex_w);
	writers++;					
	sem_signal(&mutex_w)
	if(readers == 0){				//si no hay
		//~ sem_wait(&mutex_w);			

		sem_wait(&rw_lock); 		//esperamos nuestro turno, no hay escritura concurrente todos juntos, este semaforo deberia funcionar.		
		sem_signal(&mutex_r);
		
		///escritura.
		
	}else{
		//~ sem_wait(&mutex_w);			//si hay lectores

		sem_signal(&mutex_r);
		sem_wait(&accessW);			//pedimos acceso.
									//con el acceso, no deberia haber lectores.
		sem_wait(&mutex_w);
		writers++;
		sem_signal(&mutex_w);
		sem_wait(&rw_lock);		//tomo el rw_lock
		
		///escritura.
	}
}

void RWLock :: runlock() {
	sem_wait(&mutex_r);
	sem_wait(&mutex_w);
	readers--;						//salimos
	if (readers == 0){				//si somos el ultimo en salir
		//~ sem_wait(&mutex_w);			//vemos si hay escritores esperando
		for(int i = 0; i < writers; i++){
			sem_signal(&accessW);			//si hay escritores, les cedemos el paso.
		}
		writers = 0;				//seteamos en cero escritores.
		//~ sem_signal(&mutex_w);
		sem_signal(&rw_lock);		//cedemos el lock.
	}
	sem_signal(&mutex_w);
	sem_signal(&mutex_r);
		
}

void RWLock :: wunlock() {
	sem_wait(&mutex_r);
	sem_wait(&mutex_w);
	writers--;
	if (writers == 0){
		for (int i = 0; i < readers; i++){
			sem_signal(&accessR);
		}
		readers = 0;
	}
	sem_signal(&rw_lock);
	sem_signal(&mutex_w);
	sem_signal(&mutex_r);
}
