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

typedef struct configMemo {
	char* ip_kernel;
	int puerto_kernel;
	int puerto;
	int marcos;
	int marco_size;
	int retardo_memoria;
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
//t_list* listaProcesosActivos;

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

//Prototipos
void cargarConfigFile();
void crearMemoria();
void inicializarMemoria();
void inicializarTablaDeFrames();

#endif /* MEMO_H_ */
