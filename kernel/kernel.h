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
#include "gestionDeProcesos.h"


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
	char** SEM_IDS;
	char** SEM_INIT;
	char** SHARED_VARS;
	int STACK_SIZE;
	// FALTAN AGREGAR VARIABLES SEGUN AVANCE EL TP (SEMAFOROS, QUANTUM, ETC)
}config_t;

typedef struct t_semaforo {
	int valorSemaforo;
	t_queue* colaSemaforo;
} t_semaforo;

t_config* configKernel;
config_t config;

typedef enum t_proceso_estado {
	NEW, READY, EXEC, BLOCK, EXIT
}t_proceso_estado;

//VARIABLES

fd_set master; // Conjunto maestro de file descriptor (Donde me voy a ir guardando todos los socket nuevos)
fd_set read_fds; // Conjunto temporal de file descriptors para pasarle al select()
fd_set bolsaConsolas; // Bolsa de consolas
fd_set bolsaCpus; //Bolsa de bolsaCpus
struct sockaddr_in direccionServidor; // Información sobre mi dirección
struct sockaddr_in direccionCliente; // Información sobre la dirección del cliente
int sockServ; // Socket de nueva conexion aceptada
int sockClie; // Socket a la escucha
int maxFd; // Numero del ultimo socket creado (maximo file descriptor)
int yes;
int cantBytes; // La cantidad de bytes. Lo voy a usar para saber cuantos bytes me mandaron.
int addrlen; // El tamaño de la direccion del cliente
int identidadCliente;
int fdCliente, j; // Variables para recorrer los sockets (mandar mensajes o detectar datos con el select)
int tamanioPag;
//int identificadorProceso = 0;
int identificadorProceso;
int memoria; //NECESITO GUARDAR EL FD DE MEMORIA ACA PARA LLAMARLO SIEMPRE QUE QUIERA

t_list* listaDeProcesos;
t_queue* colaNew;
t_queue* colaReady;
t_queue* colaExec;
t_queue* colaExit;
t_queue* colaCPU;
t_dictionary* tablaCompartidas;
t_dictionary* tablaSemaforos;




//Prototipos
int redondear(float numero);


#endif
