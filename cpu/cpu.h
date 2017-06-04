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
#include <parser/metadata_program.h>
#include <parser/parser.h>

//GLOBALES
struct sockaddr_in dirKernel;
int kernel;
struct sockaddr_in dirMemoria;
int memoria;
bool ejecutar;
int tamanioPaginas;
char* sentenciaPedida;
int pidInventado;

t_log *logger;
t_log *debugLogger;

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

typedef struct pedidoBytesMemoriaStruct {
	int pid;
	int	nroPagina;
	int offset;
	int tamanio;
} pedidoBytesMemoria_t;


#endif
