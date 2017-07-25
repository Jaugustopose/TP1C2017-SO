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
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include "logger.h"
#include "serializador.h"
#include "estructurasCompartidas.h"
#include "primitivas.h"
//#include <parser/parser.h>
//#include <parser/metadata_program.h>
#include <math.h>
#include <signal.h>

t_log *logger;
t_log *debugLogger;


//GLOBALES
int identidadCpu;
struct sockaddr_in dirKernel;
int kernel;
struct sockaddr_in dirMemoria;
int memoria;

//Puedo terminar
bool ejecutando;
bool terminar;

bool haySigusr1;
int overflow;
bool lanzarOverflowExep;
int tamanioPaginas;
char* sentenciaPedida;
//Espera este tiempo el CPU cuando termina de ejecutar una sentencia
int quantumSleep;
int algoritmo;

typedef enum t_tipos_cierre {
	CasoOverflow = 1,
	CasoVariableInvalida = 2,
	CasoCierreNormal = 3
}t_tipos_cierre;

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

void finalizar_programa(bool normalmente);
void enviarSolicitudBytes(int pid, int pagina, int offset, int size);
void enviarAlmacenarBytes(int pid, int pagina, int offset, int size, t_valor_variable valor);
void finalizarProgramaVariableInvalida();
void loggearFinDePrimitiva(char* primitiva);
void actualizarPC(t_PCB* pcb, t_puntero_instruccion pc);


#endif
