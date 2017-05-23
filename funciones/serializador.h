/*
 * serializador.h
 *
 *  Created on: 21/5/2017
 *      Author: utnso
 */

#ifndef SERIALIZADOR_H_
#define SERIALIZADOR_H_

#include <stdlib.h>
#include <string.h>

typedef struct {
	int id;
	int tamanio;
}t_header;

char* serializar(t_header header, char* contenidoDelMensaje) {

	char* buffer = malloc(header.tamanio + sizeof(t_header));
	memcpy(buffer, &header.id, sizeof(int)); //PRIMERO EL ID
	memcpy(buffer + 1, &header.tamanio, sizeof(int)); //SEGUNDO EL TAMAÑO
	memcpy(buffer + 2, contenidoDelMensaje, header.tamanio); // TERCERA LA DATA

	return buffer;
	free(buffer);
}



#endif /* SERIALIZADOR_H_ */
