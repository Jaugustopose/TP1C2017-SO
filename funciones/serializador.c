/*
 * serializador.c

 *
 *  Created on: 15/5/2017
 *      Author: utnso
 */

#include <stdio.h>

typedef struct header tipoHeader;

struct header {
	int id;
	int tamanio;
};

void* serializar(tipoHeader head, void* contenidoDelMensaje) {

	void* buffer = malloc(sizeof(contenidoDelMensaje) + sizeof(head));
	memcpy(buffer,&head, 4); //PRIMERO EL ID
	memcpy(buffer,&head, 4); //SEGUNDO EL TAMAÑO
	memcpy(buffer, contenidoDelMensaje, sizeof(contenidoDelMensaje)); // TERCERA LA DATA

	return buffer;
}


