/*
 * serializador.c

 *
 *  Created on: 15/5/2017
 *      Author: utnso
 */

#include <stdio.h>
#include "serializador.h"


void* serializar(t_header header, void* contenidoDelMensaje) {

	void* buffer = malloc(sizeof(contenidoDelMensaje) + sizeof(header));
	memcpy(buffer,&header, 4); //PRIMERO EL ID
	memcpy(buffer,&header, 4); //SEGUNDO EL TAMAÑO
	memcpy(buffer, contenidoDelMensaje, sizeof(contenidoDelMensaje)); // TERCERA LA DATA

	return buffer;
}

void* serializarMemoria(int codigoAccion, void* contenidoDelMensaje, int tamanioMensaje) {
	printf("tamanio mensaje: %d\n", tamanioMensaje);
	void* buffer = malloc(tamanioMensaje + sizeof(codigoAccion));
	memcpy(buffer, &codigoAccion, sizeof(codigoAccion)); //PRIMERO EL CODIGO
	memcpy(buffer + sizeof(codigoAccion), contenidoDelMensaje, tamanioMensaje); // SEGUNDO LA DATA

	return buffer;
}


