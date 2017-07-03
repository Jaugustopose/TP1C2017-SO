#ifndef consola_h
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <stdio.h>
#include "cliente-servidor.h"
#include "serializador.h"
#include <pthread.h>


typedef struct configuracion{
	char* IP_KERNEL;
	int PUERTO_KERNEL;
}config_t;

typedef struct parametrosCrearPrograma{
	int socket;
	char* programaACrear;

}param_programa;

enum opcionesUsuario {
	iniciarPrograma = 1,
	finalizarPrograma = 2,
	desconectarConsola = 3,
	limpiarMensajes = 4,
};

int identidad = 1;
t_config* configConsola;
config_t config;
char* path;
FILE* programa;
struct sockaddr_in dirKernel;

t_list* listaPIDs;
char* programasExec;



#endif
