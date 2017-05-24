#ifndef cpu_h
#define cpu_h

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <stdbool.h>
#include "primitivas.h"

//GLOBALES
struct sockaddr_in dirKernel;
int kernel;
struct sockaddr_in dirMemoria;
int memoria;
bool ejecutar;
int tamanioPaginas;
char* sentenciaPedida;
int* identidad = 2;

typedef enum {
	AccionObtenerPCB, //0
	AccionPedirSentencia, //1
	AccionFinInstruccion, //2
	AccionFinProceso //2
}t_accion;

typedef struct {
	int nroPagina;
	int offset;
	int size;
}t_pedido;



struct configuracion{
	char* IP_MEMORIA;
	char* IP_KERNEL;
	int PUERTO_MEMORIA;
	int PUERTO_KERNEL;
};

t_config* configCpu;
struct configuracion config;

//Factorizar:nucleo tiene LO MISMO
typedef struct {
	int PID;
	int contadorPaginas;
	int contadorPrograma;
	t_list* indiceCodigo;
}t_PCB;

typedef struct{
	int inicio;
	int fin;

}t_sentencia;

t_PCB* pcbNuevo;

static const char* PROGRAMA =
		"begin\n"
		"variables a, b\n"
		"a = 3\n"
		"b = 5\n"
		"a = b + 12\n"
		"end\n"
		"\n";




#endif
