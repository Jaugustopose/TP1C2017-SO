
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

t_list* tablaPaginasHeap; //elementos: pidHeap
int pidCondicion;
int solicitudCondicion;
int numeroDePaginaCondicion;


typedef struct heapMetadata{
 int size;
 _Bool isFree;
}heapMetadata;

typedef struct pagina{
  int pid;
  int nro;
  int tamDisponible;
  t_list* bloques; //elementos: bloque
}t_paginaHeap;

typedef struct pidHeap{
 int pid;
  t_list* paginas; //elementos: pagina
}t_pidHeap;



#endif /* CAPAMEMORIA_H_ */
