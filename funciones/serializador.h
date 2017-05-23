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

void* serializar(t_header header, void* contenidoDelMensaje);
void* serializarMemoria(int codigoAccion, void* contenidoDelMensaje, int tamanioMensaje);

#endif /* SERIALIZADOR_H_ */
