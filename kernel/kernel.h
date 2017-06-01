/*
 * kernel.h
 *
 *  Created on: 9/4/2017
 *      Author: utnso
 */

#ifndef KERNEL_H_
#define KERNEL_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include "deserializador.h"
#include "serializador.h"
#include "estructurasCompartidas.h"
#include "cliente-servidor.h"
#include <pthread.h>


//Estructuras y enum
typedef struct configuracion {

	int PUERTO_CONSOLA;
	int PUERTO_FS;
	int PUERTO_MEMORIA;
	int PUERTO_CPU;
	int PUERTO_KERNEL;
	char* IP_MEMORIA;
	char* IP_FS;
	int GRADO_MULTIPROG;
	// FALTAN AGREGAR VARIABLES SEGUN AVANCE EL TP (SEMAFOROS, QUANTUM, ETC)
}config_t;

typedef struct listaConDuenio {
	t_link_element *head;
	int elements_count;
	int duenio;
}t_list_con_duenio;

typedef struct pedidoBytesMemoriaStruct {
	int pid;
	int	nroPagina;
	int offset;
	int tamanio;
} pedidoBytesMemoria_t;

typedef struct pedidoAlmacenarBytesMemoriaStruct {
	pedidoBytesMemoria_t pedidoBytes;
	char* buffer;
} pedidoAlmacenarBytesMemoria_t;

typedef struct pedidoSolicitudPaginasStruct {
	int pid;
	int cantidadPaginas;
} pedidoSolicitudPaginas_t;

enum tipoDeCliente {

	soyConsola = 1,
	soyCPU = 2
};

enum tipoMensaje {

	envioScript = 1
};

t_config* configKernel;
config_t config;

//Prototipos



#endif
