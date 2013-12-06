#include "RWLock_sem.h"

using namespace std;

RWLock :: RWLock() {
	sem_init(&rw_lock, 0, 1);
	sem_init(&accessR, 0, 0);	//el primero espera.
	sem_init(&accessW, 0, 0);
	
	sem_init(&mutex,0,1);

	readers = 0;
	readers_waiting = 0;
	writers = 0;
	writers_waiting = 0;
}

RWLock :: ~RWLock() {
	sem_close(&rw_lock);
	sem_close(&accessR);
	sem_close(&accessW);
	sem_close(&mutex);
}

void RWLock :: rlock() {

	sem_wait(&mutex);				//pedimos mutex_r para evitar deadlock.
									//vemos si hay escritores escribiendo/esperando
	cout << endl << "rlock R:" << readers << " W:" << writers << endl;
	if (writers == 0 && writers_waiting == 0){				//si no hay escritores => ESTADO LECTURA
		
		//~ sem_post(&mutex_w);
		//~ sem_wait(&mutex_r);			//vemos si hay otros lectores.
		if (readers == 0){			//si soy el primero
			sem_wait(&rw_lock);		//tomo el rw_lock
			
			//*debug*/
			cout << "<*L ";
			//**/
		}
		readers++;					//sino me sumo al resto.
		//*debug*/
		cout << "<L ";
		//**/
		
		assert(writers == 0);
		assert(readers > 0);
		sem_post(&mutex);		//cedemos los mutex.

		///lectura.
		
	}else{							//hay escritores => ESTADO ESCRITURA
		//~ sem_wait(&mutex_r);			
		readers_waiting++;		 			//nos unimos a los lectores en espera.
		sem_post(&mutex);		
									//importante pedir el mutex_r antes de ceder mutex_w
		//*debug*/
		cout << "*rh* ";
		//**/
		sem_wait(&accessR);			//esperamos el cambio de estado.
									//cuando ganamos el acceso, writers deberia ser cero.
		//*debug*/
		cout << "*ruh* ";
		//**/
		sem_wait(&mutex);
		if (readers == 0){			//si soy el primero
			sem_wait(&rw_lock);		//tomo el rw_lock
			cout << "<*L ";
		}
		readers++;					//hacemos presencia		
		//*debug*/
		cout << "<L ";
		//**/		
		assert(writers == 0);
		assert(readers > 0);
		sem_post(&mutex);
		
		///lectura.
		//Ya esta el lock ocupado... < assert! readers > 0 && writers == 0
		

	}
}

void RWLock :: wlock() {

	sem_wait(&mutex);				//vemos si hay lectores leyendo/esperando

	cout << endl << "wlock R:" << readers << " W:" << writers << endl;
	if(readers == 0 && readers_waiting == 0){				//si no hay
		//~ sem_wait(&mutex_w);			
		
		sem_post(&mutex);
		sem_wait(&rw_lock); 		//esperamos nuestro turno, no hay escritura concurrente todos juntos, este semaforo deberia funcionar.		
		
		//*debug*/
		cout << "<*E ";
		sem_wait(&mutex);
		writers++;
		sem_post(&mutex);
		//**/
		assert(readers == 0);
		///escritura.
		
	}else{
		//~ sem_wait(&mutex_w);			//si hay lectores
		writers_waiting++;
		sem_post(&mutex);
		//*debug*/
		cout << "*wh* ";
		//**/
		sem_wait(&accessW);			//pedimos acceso.
									//con el acceso, no deberia haber lectores.
		//*debug*/
		cout << "*wuh* ";
		//**/
		sem_wait(&mutex);
		writers++;
		sem_post(&mutex);
		
		sem_wait(&rw_lock);		//tomo el rw_lock
		
		assert(readers == 0);
		///escritura.
		//*debug*/
		cout << "<*E ";
		//**/
	}
	
	//Ya esta el lock ocupado... < assert! readers == 0 && g == 1
	
}

void RWLock :: runlock() {

	sem_wait(&mutex);
	cout << endl << "runlock R:" << readers << " W:" << writers << endl;
	readers--;						//salimos
	if (readers == 0){				//si somos el ultimo en salir
		//~ sem_wait(&mutex_w);			//vemos si hay escritores esperando
		if (writers_waiting > 0){
			for(int i = 0; i < writers_waiting; i++){
				sem_post(&accessW);			//si hay escritores, les cedemos el paso.
			}
			writers_waiting = 0;
			//writers = 0;				//seteamos en menos un escritores para que nuevos lectores vean que es NO su turno.
		}
		//~ sem_post(&mutex_w);
		//*debug*/
		cout << "L*> ";
		//**/		
		sem_post(&rw_lock);		//cedemos el lock.

	} else {
		//*debug*/
		cout << "L> ";
		//**/	
	}
	sem_post(&mutex);

}

void RWLock :: wunlock() {
	
	sem_wait(&mutex);
	cout << endl << "wunlock R:" << readers << " W:" << writers << endl;
	writers--;
	if (writers == 0){
		if(readers_waiting > 0){
			for (int i = 0; i < readers_waiting; i++){
				sem_post(&accessR);
			}
			readers_waiting = 0;
			//readers = -1;
		}
	}	
	//*debug*/
	cout << "E*> ";
	//**/
	
	sem_post(&rw_lock);
	sem_post(&mutex);
}
