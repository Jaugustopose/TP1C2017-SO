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

int serializar_lista(char* destino, t_list* fuente, int pesoElemento) {
	int i, offset = 1;
	destino[0] = list_size(fuente);
	for (i = 0; i < destino[0]; i++) {
		memcpy(destino + offset, list_get(fuente, i), pesoElemento);
		offset += pesoElemento;
	}
	return offset;
}

void* serializar_PCB(t_PCB* pcb, int sock, int codigoAccion) {
	char* pcbSerializado;
	int desplazamiento = 0;

	memcpy(pcbSerializado + desplazamiento, &(pcb->PID),sizeof(int));
	desplazamiento = desplazamiento + sizeof(int);

	memcpy(pcbSerializado + desplazamiento, &(pcb->contadorPrograma), sizeof(int));
	desplazamiento = desplazamiento + sizeof(int);

	memcpy(pcbSerializado + desplazamiento, &(pcb->cantidadPaginas), sizeof(int));
	desplazamiento = desplazamiento + sizeof(int);

	printf("serializacion: %.*s\n",pcbSerializado + desplazamiento ,pcbSerializado);
	//desplazamiento = desplazamiento + serializar_lista(pcbSerializado + desplazamiento, pcb->indiceCodigo, sizeof(t_sentencia));

	//desplazamiento = desplazamiento + deserializar_stack(pcbNuevo->stackPointer, pcbSerializado + desplazamiento);


	void* buffer = malloc(desplazamiento + sizeof(codigoAccion));
	memcpy(buffer, &codigoAccion, sizeof(codigoAccion)); //PRIMERO EL CODIGO
	memcpy(buffer + sizeof(codigoAccion), pcbSerializado, sizeof(codigoAccion) + desplazamiento ); // SEGUNDO LA DATA

	send(sock, &pcbSerializado, sizeof(codigoAccion) + desplazamiento, 0);

	return 0;
}


