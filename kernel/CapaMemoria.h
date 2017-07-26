
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
#include <math.h>

#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>

#include "parser/parser.h"

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

bool solicitudValida(int espacioSolicitado);
t_pidHeap* getPID(int pid);
t_paginaHeap* getPagina(t_pidHeap* pidElement, int nroPag);
t_bloque* getBloque(t_paginaHeap* pagina, int indice);
int getLastNroPag(int pid);
int solicitarPagina(int pid);
t_paginaHeap* getPaginaConEspacio(t_pidHeap* pidElement, int pid, int espacio);
t_bloque* getBloqueConEspacio(t_paginaHeap* pagina, int espacio);
t_puntero alocar(int pid, int espacio);
t_puntero alocarMemoria(int espacioSolicitado, int pid);
void bloquesDestroyer(t_bloque* bloque);
void paginasDestroyer(t_paginaHeap* pagina);
void liberarPaginaEstructura(t_paginaHeap* paginaALiberar, t_pidHeap* pidElement);
bool bloquesTodosFree(t_paginaHeap* pagina);
int calcularIndiceBloque(t_paginaHeap* pagina, int offset);
int liberarMemoria(t_puntero puntero, int pid, int cantPaginasCodigo);
void liberarPagina(t_paginaHeap* pagina, int puntero, int cantPaginasCodigo);
int32_t defragmentar(t_paginaHeap* pagina);



#endif /* CAPAMEMORIA_H_ */
