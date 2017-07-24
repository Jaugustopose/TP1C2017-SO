
#ifndef CAPAMEMORIA_H_
#define CAPAMEMORIA_H_

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
#include <math.h>
#include "kernel.h"


t_list *listaPidHEAP; //elementos: pidHeap

typedef struct heapMetadata{
	int32_t size;
 _Bool isFree;
}t_heapMetadata;

typedef struct bloque
{
 int32_t size;
 _Bool isFree;
 int32_t indice;
}t_bloque;

typedef struct pagina{
 int32_t pid;
 int32_t nro;
 int32_t tamDisponible;
 t_list *bloques; //elementos: bloque
}t_paginaHeap;

typedef struct pidHeap{
int32_t pid;
 t_list *paginas; //elementos: pagina
}t_pidHeap;



#endif /* CAPAMEMORIA_H_ */
