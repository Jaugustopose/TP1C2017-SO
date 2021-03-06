/*
 * memo.h
 *
 *  Created on: 9/4/2017
 *      Author: utnso
 */
#ifndef MEMO_H_
#define MEMO_H_
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include "commons/config.h"
#include "cliente-servidor.h"
#include <pthread.h>
#include <commons/collections/list.h>
#include <stdbool.h>
#include "estructurasCompartidas.h"
#include <commons/log.h>
#include <commons/txt.h>
#include <signal.h>


typedef struct configMemo {
	char* ip_kernel;
	int32_t puerto_kernel;
	int32_t puerto;
	int32_t marcos;
	int32_t marco_size;
	int32_t entradas_cache;
	int32_t cache_x_proc;
	int32_t retardo_memoria;
} config_t;

/**
 * No incluimos el atributo nroFrame ya que armamos
 * esta estructura para incluirla en un array. Como
 * los nros de frame son continuos y contiguos sin
 * repetirse y 0-based, usaremos la posición del array
 * para referir al frame, lo que nos ahorrará espacio
 */
typedef struct tablaPaginaStruct {
	int pid;
	int	nroPagina;
} tablaPagina_t;

t_config* configMemo;
config_t config;

char* memoria;
int tamanioMemoria;
int tamanioTablaPagina;
int cantMarcosOcupaTablaPaginas;
t_list** overflow;
int retardoMemoria;
int stack_size;//lo recibe del kernel
char* cache;
int tamanioCache;
t_list* entradasLibresCache;
t_list* entradasOcupadasCache;
//t_list* listaProcesosActivos;
t_log *memoLogger;
t_log *memoConsoleLogger;
char* directorioOutputMemoria = "output";
char* memoriaLogFileName = "memoria";
pthread_mutex_t lockMemoria;
pthread_mutex_t lockTablaPaginas;
pthread_mutex_t lockColisiones;


enum accionConsolaMemoria {
	retardo = 1,
	dumpCache = 2,
	dumpEstructurasDeMemoria = 3,
	dumpMemoriaCompleta = 4,
	dumpMemoriaProceso = 5,
	flushCache = 6,
	sizeMemoria = 7,
	sizePid = 8
};

typedef struct parametrosHiloDedicado{
	int socketClie;
	tablaPagina_t* tablaPaginasInvertida;
}paramHiloDedicado;

typedef struct administrativaCache {
	int32_t pid;
	int32_t nroPagina;
	void* contenido;

}entradaCache_t;

static void entrada_destroyer(entradaCache_t *self) {
    free(self->contenido);
    free(self);
}



//Prototipos
void cargarConfigFile();
void crearMemoria();
void inicializarMemoria();
void inicializarTablaDeFrames();
int buscarMarco(int pid, int nroPagina, tablaPagina_t* tablaPaginasInvertida);
unsigned int calcularPosicion(int pid, int num_pagina);
void inicializarOverflow(int cantidad_de_marcos);
void agregarSiguienteEnOverflow(int pos_inicial, int nro_frame);
int buscarEnOverflow(int indice, int pid, int pagina, tablaPagina_t* tablaPaginasInvertida);
void borrarDeOverflow(int pos_inicial, int frame);
int esMarcoCorrecto(int pos_candidata, int pid, int pagina, tablaPagina_t* tablaPaginasInvertida);
bool estaElMarcoReservado(int marcoBuscado, int cantPaginasSolicitadas, int marcosSolicitados[][2]);
int liberarPaginaPid(int pid, int nroPagina, tablaPagina_t* tablaPaginasInvertida);
int configurarRetardoMemoria();
void realizarDumpEstructurasDeMemoria(tablaPagina_t* tablaPaginasInvertida);
void realizarDumpContenidoMemoriaCompleta(tablaPagina_t* tablaPaginasInvertida);
void realizarDumpContenidoProceso(tablaPagina_t* tablaPaginasInvertida);
void obtenerSizeMemoria(tablaPagina_t* tablaPaginasInvertida);
void obtenerSizePid(tablaPagina_t* tablaPaginasInvertida);


#endif /* MEMO_H_ */
