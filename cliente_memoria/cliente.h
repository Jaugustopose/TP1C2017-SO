/*
 * cliente_prueba.h
 *
 *  Created on: 21/5/2017
 *      Author: utnso
 */

#ifndef CLIENTE_H_
#define CLIENTE_H_

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

#endif /* CLIENTE_H_ */
