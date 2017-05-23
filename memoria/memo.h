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
//#include "memo.cfg"

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

typedef struct pedidoBytesMemoriaStruct {
	int pid;
	int	nroPagina;
	int offset;
	int tamanio;
} pedidoBytesMemoria_t;

typedef struct pedidoAlmacenarBytesMemoriaStruct {
	pedidoBytesMemoria_t pedidoBytes;
	char* buffer;
} pedidoAlmacenarBytesMemoria_t;

typedef struct pedidoSolicitudPaginasStruct {
	int pid;
	int cantidadPaginas;
} pedidoSolicitudPaginas_t;

t_config* configMemo;
config_t config;

char* memoria;
int tamanioMemoria;
int tamanioTablaPagina;
enum accionMemoria {
	inicializarProgramaAccion = 1,
	solicitarPaginasAccion = 2,
	almacenarBytesAccion = 3,
	solicitarBytesAccion = 4
};

//Prototipos
void cargarConfigFile();
void crearMemoria();
void inicializarMemoria();
void inicializarTablaDeFrames();

#endif /* MEMO_H_ */
