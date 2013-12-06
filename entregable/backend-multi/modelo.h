#ifndef __MODELO_H__
#define __MODELO_H__

#include <string>
#include <jugador.h>
#include <evento.h>
#include <tiro.h>
#include <queue>
//rwlock
#include "RWLock_sem.h"

enum EstadoJuego {SETUP, DISPAROS, FINALIZADO};

class Modelo {
    public:
        Modelo(int njugadores, int tamtablero, int tamtotalbarcos);
        ~Modelo();
        int     agregarJugador(std::string nombre);		/* agrega el jugador y devuelve el id */
		error	ubicar(int t_id, int * xs, int *  ys, int tamanio);	/* ubica el barco para el jugador t_id */
		error	borrar_barcos(int t_id);
	    error   empezar();								/* pasa a la fase de tiroteo */
		error   finalizar();							/* finaliza la fase de tiroteo */
        error   reiniciar();							/* reinicia el score y los jugadores, pasa a la fase de ubicacion */
        error   ack(int s_id);
        bool    termino();
		error   quitarJugador(int s_id);                /* quita un jugador del juego */
		int     apuntar(int s_id, int t_id, int x, int y, int *eta);    /* s_id apunta a (x,y) del jugador t_id */
        int     tocar(int s_id, int t_id);    /* s_id toca a (x, y) del jugador t_id */
        int     dame_eta(int s_id);
		Evento	actualizar_jugador(int t_id);	/* devuelve el proximo evento del jugador */
        int     hayEventos(int s_id);
        Evento  dameEvento(int s_id);
		
#ifdef DEBUG
		void	print();
#endif
	private:
		error	_borrar_barcos(int t_id);
	    error   _empezar();								
		error   _finalizar();							
		int						cantidad_jugadores;				/* cantidad de jugadores inscriptos */
		int						jugadores_listos;				/* cantidad de jugadores ya ubicados */
		Jugador					**jugadores;					/* Jugadores */
		Tiro					*tiros;							/* estado de los tiros */
		std::queue<Evento>		*eventos;						/* arreglo de colas de eventos por jugador */
		EstadoJuego				jugando;						/* si se esta jugando (fase tiroteo) o ubicando */
	
		friend class	Jsonificador;					/* para poder acceder a los elementos */
		
		/** ACA VAN LOS LOCKS; HAY QUE PROTEGER EL MODELO Y TENER CUIDADO CON EL JSONIFICADOR QUE ES FRIEND 
		 * 
		 * RWLock lock1,lock2...
		*/
		//~ RWLock *lock_cant_jugadores;
		//~ RWLock *lock_jugadores_listos;
		RWLock *lock_jugadores;
		RWLock *lock_estado_jugadores;
		RWLock *lock_eventos;
		RWLock *lock_estado_eventos;
		RWLock *lock_jugando;
		
};


#endif /* __MODELO_H__ */
