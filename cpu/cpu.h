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
#include <parser/parser/parser.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>

//GLOBALES
struct sockaddr_in dirKernel;
int kernel;
struct sockaddr_in dirMemoria;
int memoria;
int ejecutar;
int tamanioPaginas;
char* sentencia;
int* identidad = 2;

typedef enum {
	AccionObtenerPCB, //0
	AccionPedirSentencia //1
}t_accion;

typedef struct {
	int nroPagina;
	int offset;
	int size;
}t_solicitud;



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
	int offset_inicio;
	int longitud;

}t_sentencia;

t_PCB* pcbNuevo;




#endif
