/*
 * kernel.h
 *
 *  Created on: 9/4/2017
 *      Author: utnso
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdbool.h>
#include <commons/config.h>
#include <commons/string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "cliente-servidor.h"

<<<<<<< HEAD:kernel/kernel.h
struct configuracion{
=======

struct configuracion {
>>>>>>> d0a32bed51155ca9effd44b78b9f93f21e5224a3:kernel/kernel.h

	int PUERTO_CONSOLA;
	int PUERTO_FS;
	int PUERTO_MEMORIA;
	int PUERTO_CPU;
	int PUERTO_KERNEL;
	char* IP_MEMORIA;
	char* IP_FS;
	// FALTAN AGREGAR VARIABLES SEGUN AVANCE EL TP (SEMAFOROS, QUANTUM, ETC)
};
struct configuracion config;


#endif
