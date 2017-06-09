#ifndef cpu_h
#define cpu_h

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
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
#include "logger.h"
#include "serializador.h"
#include "estructurasCompartidas.h"
#include "primitivas.h"
#include <parser/parser.h>
#include <parser/metadata_program.h>
#include <math.h>


//OJO: El numero corresponde al numero del proceso con el que se comunica
//Cambiar un numero SOLO ACA implica romper la comunicacion
enum accionCPU {
	solicitarPaginasAccion = 2,
	solicitarBytesAccion = 4,
	accionContinuarProceso = 5,
	obtenerTamanioPaginas = 6,
	accionObtenerPCB = 7,
	accionFinInstruccion = 8,
	accionFinProceso = 9,
	accionImprimirTextoKernel = 10, //primitiva kernel
	accionImprimirVariableKernel = 11, //primitiva kernel
	accionAsignarValorCompartidaKernel = 12, //primitiva kernel
	accionPedirValorCompartidaKernel = 13, //primitiva kernel
	accionQuantumInterrumpido = 14, //signal
	almacenarBytesAccion = 16, // memoria (asignar, primitiva)
	accionException = 17, //para overflow
	accionError = 18

};

t_log *logger;
t_log *debugLogger;

//GLOBALES
struct sockaddr_in dirKernel;
int kernel;
struct sockaddr_in dirMemoria;
int memoria;
bool ejecutar;
bool finalizarEjec;
bool salteaCircuitoConGoTo;
int overflow;
bool lanzarOverflowExep;
int tamanioPaginas;
char* sentenciaPedida;
//Espera este tiempo el CPU cuando termina de ejecutar una sentencia
int quantumSleep;


struct configuracion{
	char* IP_MEMORIA;
	char* IP_KERNEL;
	int PUERTO_MEMORIA;
	int PUERTO_KERNEL;
};

t_config* configCpu;
struct configuracion config;

t_PCB* pcbNuevo;
t_stack* stack;
int cantidadPagCodigo;




#endif
