/*
 * serializador.h
 *
 *  Created on: 21/5/2017
 *      Author: utnso
 */

#ifndef SERIALIZADOR_H_
#define SERIALIZADOR_H_
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <stdlib.h>
#include <string.h>
#include "estructurasCompartidas.h"

typedef struct {
	int id;
	int tamanio;
}t_header;


/*char* serializar(t_header header, char* contenidoDelMensaje) {

	char* buffer = malloc(header.tamanio + sizeof(t_header));
	memcpy(buffer, &header.id, sizeof(int)); //PRIMERO EL ID
	memcpy(buffer + 1, &header.tamanio, sizeof(int)); //SEGUNDO EL TAMAÑO
	memcpy(buffer + 2, contenidoDelMensaje, header.tamanio); // TERCERA LA DATA

	return buffer;
	free(buffer);
}*/


void* serializar(t_header header, void* contenidoDelMensaje);
void* serializarMemoria(int codigoAccion, void* contenidoDelMensaje, int tamanioMensaje);

#endif /* SERIALIZADOR_H_ */
