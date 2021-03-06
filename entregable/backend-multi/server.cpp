//
//  server.c
//  Batalla Naval
//
//  Created by Federico Raimondo on 4/24/13.
//  Copyright (c) 2013 ar.dc.uba.so. All rights reserved.
//

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h> 
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <modelo.h>
#include <decodificador.h>
#include <globales.h>

//pthread
#include <pthread.h>

#define MAX_MSG_LENGTH 4096
#define MAX_JUGADORES 100
#define MAX_CONTROLADORES 100

/* Setea un socket como no bloqueante */
int no_bloqueante(int fd) {
    int flags;
    /* Toma los flags del fd y agrega O_NONBLOCK */
    if ((flags = fcntl(fd, F_GETFL, 0)) == -1 )
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}


/* Variables globales del server */
int sock, c_sock;										// Sockets donde se escuchan las conexiones entrantes para jugadores/controladores
struct sockaddr_in name, remote;						// Direcciones
struct sockaddr_in c_name, c_remote;					// Direcciones
int s[MAX_JUGADORES];									// Sockets de los jugadores
int ids[MAX_JUGADORES];									// Ids de los jugadores
Modelo * model = NULL;									// Puntero al modelo del juego
Decodificador *decoders[MAX_JUGADORES];					// Decodificadores de los jugadores
Decodificador *deco_controladores[MAX_CONTROLADORES];	// Decodificadores de los controladores

//globales de los threads de control.
pthread_t control_threads[MAX_CONTROLADORES]; 
int c_tids[MAX_CONTROLADORES];			


int n, tamanio, tamanio_barcos;		// Variables de configuracion del juego.


/* Resetea el juego */
void reset() {
	if (model != NULL) {
		delete model;
	}
	model = new Modelo(n, tamanio, tamanio_barcos);

	for(int i = 0; i < MAX_CONTROLADORES; ++i){
		if (deco_controladores[i] != NULL) {
			delete deco_controladores[i];
		}	
		deco_controladores[i] = new Decodificador(model);
	}
	
	for(int i = 0; i < n; ++i){
		if (decoders[i] != NULL) {
			delete decoders[i];
		}
		decoders[i] = new Decodificador(model);
	}
}

/* Socket de comunicación de *//**los controladores */
int s_controladores[MAX_CONTROLADORES];
/* Para anteder al controlador */
void atender_controlador(int i) {
	char buf[MAX_MSG_LENGTH];
	int recibido;
	std::string resp;
	recibido = recv(s_controladores[i], buf, MAX_MSG_LENGTH, 0);
	if (recibido < 0) {
		perror("Recibiendo ");		
	} else if (recibido > 0) {
		buf[recibido]='\0';
		char * pch = strtok(buf, "|");
		while (pch != NULL) {
			//Ejecutar y responder
			resp = deco_controladores[i]->decodificar(pch);
			send(s_controladores[i],resp.c_str(), resp.length() +1, 0);
			pch = strtok(NULL, "|");
		}
	}
}


/* Para atender al i-esimo jugador */
void atender_jugador(int i) {
	char buf[MAX_MSG_LENGTH];
	int recibido;
	std::string resp;
	recibido = recv(s[i], buf, MAX_MSG_LENGTH, 0);
	if (recibido < 0) {
		perror("Recibiendo ");
		
	} else if (recibido > 0) {
		buf[recibido]='\0';
		// Separo los mensajes por el caracter |
		char * pch = strtok(buf, "|");
		while (pch != NULL) {
			
			// No muestro por pantalla los NOP, son muchos
			if (strstr(pch, "Nop") == NULL) {
				printf("Recibido: %s\n", pch);
			}
			
			//Decodifico el mensaje y obtengo una respuesta
			resp = decoders[i]->decodificar(pch);
			
			// Si no se cual es el ID de este jugador, trato de obtenerlo de la respuesta
			if (ids[i] == -1) {
				ids[i] = decoders[i]->dameIdJugador(resp.c_str());
			}
			
			// Envio la respuesta
			send(s[i],resp.c_str(), resp.length() +1, 0);
			
			// No muestro por pantalla los NOP, son muchos
			if (strstr(pch, "Nop") == NULL) {
				printf("Resultado %s\n", resp.c_str());
			}
			
			// Si ya se cual es el jugador
			if (ids[i] != -1) {
				// Busco si hay eventos para enviar y los mando
				bool hayEventos = model->hayEventos(ids[i]);
				while(hayEventos) {
					resp = decoders[i]->encodeEvent(ids[i]);
					printf("Enviando evento %s", resp.c_str());
					send(s[i],resp.c_str(), resp.length() +1, 0);
					hayEventos = model->hayEventos(ids[i]);
				}
			}
			pch = strtok(NULL, "|");
		}
	}
}

/* Acepta todas las conexiones entrantes */

void *atencion_p(void* arg) {

	int jugador = *((int *) arg);
	
	//se quede atendiendolo
	bool sale = false;
	while(!sale){
		atender_jugador(jugador);
		sale = model->termino();
	}
	
	pthread_exit(NULL);
	return NULL;
}

void *atencion_c(void* arg) {
	int controlador = *((int *) arg);
	
	atender_controlador(controlador);
	
	close(s_controladores[controlador]);
	pthread_exit(NULL);
	return NULL;
}

void *accept_c(void*) {
	int t;
	
	for(int controlador = 0; controlador < MAX_CONTROLADORES; ++controlador){
		c_tids[controlador] = controlador;
		t = sizeof(c_remote);
		if ((s_controladores[controlador] = accept(c_sock, (struct sockaddr*) &c_remote, (socklen_t*) &t)) == -1) {
			perror("aceptando la conexión entrante");
			exit(1);
		}	
		int flag = 1;
		setsockopt(s_controladores[controlador],            /* socket affected */
				IPPROTO_TCP,     							/* set option at TCP level */
				TCP_NODELAY,								/* name of option */
				(char *) &flag,  							/* the cast is historical */
				sizeof(int));    							/* length of option value */		
		
		pthread_create(&control_threads[controlador],NULL,atencion_c,&c_tids[controlador]);
	}
	pthread_exit(NULL);
	//lanza un thread por cada conexion entrante.
	return NULL;
}

/*
 * Recibe 4 parametros:
 * argv[1]: Puerto
 * argv[2]: Cantidad de jugadores (N)
 * argv[3]: Tamanio del tablero
 * argv[4]: Tamanio total de los barcos
 */
int main(int argc, char * argv[]) {
	if (argc < 5) {
		printf("Faltan parametros\n");
		printf("Se espera ./server puerto jugadores tamanio_tablero tamanio_barcos\n");
		exit(1);
	}
	int port = atoi(argv[1]);
	n = atoi(argv[2]);
	tamanio = atoi(argv[3]);
	tamanio_barcos = atoi(argv[4]);
	
	inicializar();
	int port_controlador = CONTROLLER_PORT;
	
	for(int r = 0; r < MAX_JUGADORES; r++) decoders[r] = NULL;
	
	printf("Escuchando en el puerto %d - controlador en %d\n", port, port_controlador);
	printf("Jugadores %d - Tamanio %d - Tamanio Barcos %d\n", n, tamanio, tamanio_barcos);
	reset();

	
	
	/* Crear socket sobre el que se lee: dominio INET, protocolo TCP (STREAM). */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("abriendo socket");
		exit(1);
	}
	/* Crear nombre, usamos INADDR_ANY para indicar que cualquiera puede enviar aquí. */
	name.sin_family = AF_INET;
	name.sin_addr.s_addr = INADDR_ANY;
	name.sin_port = htons(port);
	if (bind(sock, (const struct sockaddr*) &name, sizeof(name))) {
		perror("binding socket");
		exit(1);
	}
	
	/* Escuchar en el socket y permitir n conexiones en espera. */
	if (listen(sock, n) == -1) {
		perror("escuchando");
		exit(1);
	}
	
	/* Crear socket sobre el que se lee*//** PARA CONTROLADORES*//*: dominio INET, protocolo TCP (STREAM). */
	c_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (c_sock < 0) {
		perror("abriendo socket");
		exit(1);
	}
	/* Crear nombre, usamos INADDR_ANY para indicar que cualquiera puede enviar aquí. */
	c_name.sin_family = AF_INET;
	c_name.sin_addr.s_addr = INADDR_ANY;
	c_name.sin_port = htons(port_controlador);
	if (bind(c_sock, (const struct sockaddr*) &c_name, sizeof(c_name))) {
		perror("binding socket");
		exit(1);
	}
	
	/* Escuchar en el socket y permitir MAX_CONTROLADORES conexiones en espera. */
	if (listen(c_sock, MAX_CONTROLADORES) == -1) {
		perror("escuchando");
		exit(1);
	}

	//~ pthread_t control_threads[MAX_CONTROLADORES]; 
	//~ int c_tids[MAX_CONTROLADORES];
	for(int i = 0; i < MAX_CONTROLADORES; ++i){
		c_tids[i] = -1;
	}
	
	pthread_t thread_atencion_controladores;
	pthread_create(&thread_atencion_controladores,NULL,accept_c,NULL);
	
	pthread_t threads[MAX_JUGADORES]; 
	int tids[MAX_JUGADORES];
	
	//~ for(int tid = 0; tid < n; ++tid){
		//~ tids[tid] = tid;
		//~ pthread_create(&threads[tid],NULL,accept_p,&tids[tid]);
	//~ } 
		
	int t;	
	for (int jugador = 0; jugador < n; ++jugador){
		t = sizeof(remote);
		if ((s[jugador] = accept(sock, (struct sockaddr*) &remote, (socklen_t*) &t)) == -1) {
			perror("aceptando la conexión entrante");
			exit(1);
		}
		ids[jugador] = -1;
		int flag = 1;
		setsockopt(s[jugador],					/* socket affected */
				IPPROTO_TCP,				   /* set option at TCP level */
				TCP_NODELAY,     /* name of option */
				(char *) &flag,  /* the cast is historical */
				sizeof(int));    /* length of option value */
		
		tids[jugador] = jugador;	
		pthread_create(&threads[jugador],NULL,atencion_p,&tids[jugador]);
	}
		
	for (int tid = 0; tid < n; ++tid){
		pthread_join(threads[tid], NULL);
	}
	
	for (int tid = 0; tid < MAX_CONTROLADORES && c_tids[tid] != -1; ++tid){
		pthread_join(control_threads[tid], NULL);
	}
	
    printf("\nTermino el juego.\n");
    printf("%s",model->damePuntajes().c_str());
	printf("\n cerrando...\n ");
    
	for (int i = 0; i < n; i++) {
		close(s[i]);
	}
	
	close(sock);
	close(c_sock);
	return 0;
}
