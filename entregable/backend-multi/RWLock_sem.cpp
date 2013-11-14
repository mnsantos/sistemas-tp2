#include "RWLock_sem.h"

using namespace std;

RWLock :: RWLock() {
	sem_init(&rw_lock, 0, 1);
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
	if (readers == -1){			
		readers++;
		sem_wait(&rw_lock);		
		//*debug*/
		//~ cout << "<*L ";
		//**/
		readers++;					
		//*debug*/
		//~ cout << "<L ";
		//**/
		sem_post(&mutex_r);		//cedemos los mutex.
		sem_post(&mutex_w);
		
		///lectura.
		
	}else if (writers == 0){				//si no hay escritores => ESTADO LECTURA
		
		//~ sem_post(&mutex_w);
		//~ sem_wait(&mutex_r);			//vemos si hay otros lectores.
		if (readers == 0){			//si soy el primero
			sem_wait(&rw_lock);		//tomo el rw_lock
			
			//*debug*/
			//~ cout << "<*L ";
			//**/
		}
		readers++;					//sino me sumo al resto.
		//*debug*/
		//~ cout << "<L ";
		//**/
		sem_post(&mutex_r);		//cedemos los mutex.
		sem_post(&mutex_w);
		
		///lectura.

		
	}else{							//hay escritores => ESTADO ESCRITURA
		//~ sem_wait(&mutex_r);			
		readers++;					//nos unimos a los lectores en espera.
		sem_post(&mutex_r);		
		sem_post(&mutex_w);		//importante pedir el mutex_r antes de ceder mutex_w
		sem_wait(&accessR);			//esperamos el cambio de estado.
									//cuando ganamos el acceso, writers deberia ser cero.
		sem_wait(&mutex_r);
		if (readers == -1){			//si soy el primero
			readers++;
			sem_wait(&rw_lock);		//tomo el rw_lock
		}
		readers++;					//hacemos presencia		
		//*debug*/
		//~ cout << "<L ";
		//**/		
		sem_post(&mutex_r);
		
		///lectura.
		//Ya esta el lock ocupado... < assert! readers > 0 && writers == 0
		

	}
}

void RWLock :: wlock() {
	sem_wait(&mutex_r);				//vemos si hay lectores leyendo/esperando
	sem_wait(&mutex_w);
	if(writers == -1) writers++;
	writers++;					
	sem_post(&mutex_w);
	if(readers == 0){				//si no hay
		//~ sem_wait(&mutex_w);			
		
		sem_post(&mutex_r);
		sem_wait(&rw_lock); 		//esperamos nuestro turno, no hay escritura concurrente todos juntos, este semaforo deberia funcionar.		
		
		//*debug*/
		//~ cout << "<*E ";
		//**/
		
		///escritura.
		
	}else{
		//~ sem_wait(&mutex_w);			//si hay lectores

		sem_post(&mutex_r);
		sem_wait(&accessW);			//pedimos acceso.
									//con el acceso, no deberia haber lectores.
		sem_wait(&mutex_w);
		if(writers == -1) writers++;
		writers++;
		sem_post(&mutex_w);
		
		sem_wait(&rw_lock);		//tomo el rw_lock
		
		///escritura.
		//*debug*/
		//~ cout << "<*E ";
		//**/
	}
	
	//Ya esta el lock ocupado... < assert! readers == 0 && g == 1
	
}

void RWLock :: runlock() {
	sem_wait(&mutex_r);
	sem_wait(&mutex_w);
	readers--;						//salimos
	if (readers == 0){				//si somos el ultimo en salir
		//~ sem_wait(&mutex_w);			//vemos si hay escritores esperando
		if (writers > 0){
			for(int i = 0; i < writers; i++){
				sem_post(&accessW);			//si hay escritores, les cedemos el paso.
			}
			writers = -1;				//seteamos en menos un escritores para que nuevos lectores vean que es NO su turno.
		}
		//~ sem_post(&mutex_w);
		//*debug*/
		//~ cout << "L*> ";
		//**/		
		sem_post(&rw_lock);		//cedemos el lock.

	}
	//*debug*/
	//~ cout << "L> ";
	//**/	
	
	sem_post(&mutex_w);
	sem_post(&mutex_r);

		
}

void RWLock :: wunlock() {
	
	//Muestran >
	
	sem_wait(&mutex_r);
	sem_wait(&mutex_w);
	writers--;
	if (writers == 0){
		if(readers > 0){
			for (int i = 0; i < readers; i++){
				sem_post(&accessR);
			}
			readers = -1;
		}
	}	
	//*debug*/
	//~ cout << "E*> ";
	//**/
	
	sem_post(&rw_lock);
	sem_post(&mutex_w);
	sem_post(&mutex_r);
}
