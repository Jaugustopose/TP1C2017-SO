/*
 * serializador.h
 *
 *  Created on: 21/5/2017
 *      Author: utnso
 */

#ifndef SERIALIZADOR_H_
#define SERIALIZADOR_H_

typedef struct {
	int id;
	int tamanio;
}t_header;

void* serializar(t_header header, void* contenidoDelMensaje) {

	void* buffer = malloc(sizeof(contenidoDelMensaje) + sizeof(header));
	memcpy(buffer,&header.id, 4); //PRIMERO EL ID
	memcpy(buffer,&header.tamanio, 4); //SEGUNDO EL TAMAÑO
	memcpy(buffer, contenidoDelMensaje, sizeof(contenidoDelMensaje)); // TERCERA LA DATA

	return buffer;
}



#endif /* SERIALIZADOR_H_ */
