/*
 * kernel.h
 *
 *  Created on: 9/4/2017
 *      Author: utnso
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include <commons/config.h>
#include <commons/string.h>
#include <string.h>

struct configuracion {


	int PUERTO_CONSOLA;
	int PUERTO_FS;
	int PUERTO_MEMORIA;
	int PUERTO_CPU;
	int PUERTO_KERNEL;
	char* IP_MEMORIA;
	char* IP_FS;
	int GRADO_MULTIPROG;
	// FALTAN AGREGAR VARIABLES SEGUN AVANCE EL TP (SEMAFOROS, QUANTUM, ETC)
};
t_config* configKernel;
struct configuracion config;

typedef struct {

	int PID;
	int contadorPaginas;
	int exit_code;
}t_PCB;

#endif
