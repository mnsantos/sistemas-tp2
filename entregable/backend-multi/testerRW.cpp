#include <iostream>
#include "RWLock.h"
#include <unistd.h>

int main(int argc, char **argv)
{
	RWLock lock;
	int variable_a_leer_escribir = 0;
	int cant_lectores = 1
	int cant_escritores = 1
	
	for (int i = 0; i < cant_lectores; i++);
		pid_t pid = fork();
		if (pid == 0) {
			lector();
			break;
		}
    }
    if (pìd != 0){	
	
		for (int i = 0; i < cant_escritores; i++){
			pid_t pid = fork();
			if (pid == 0) {
				escritor();
				break;
			}
		}	
	}
	
	if (pìd != 0){	
	
	
		//~ sincronizar
		//~ for each
		wait(&status);
	
	
	
	}else{
		
		//~ exit();
	}
	
	
	return 0;
}
