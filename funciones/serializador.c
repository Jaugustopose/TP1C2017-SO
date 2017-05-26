/*
 * serializador.c

 *
 *  Created on: 15/5/2017
 *      Author: utnso
 */

#include <stdio.h>
#include <sys/socket.h>
#include "serializador.h"
#include "estructurasCompartidas.h"


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

int serializar_pedido(char* destino, t_pedido* origen) {
	memcpy(destino, origen, sizeof(t_pedido));
	return sizeof(t_pedido);
}

int serializar_sentencia(char* destino, t_sentencia* origen) {
	memcpy(destino, origen, sizeof(t_sentencia));
	return sizeof(t_sentencia);
}

int serializar_lista(char* destino, t_list* origen, int pesoElemento) {
	int i, offset = 1;
	destino[0] = list_size(origen);
	for (i = 0; i < destino[0]; i++) {
		memcpy(destino + offset, list_get(origen, i), pesoElemento);
		offset += pesoElemento;
	}
	return offset;
}

int serializar_int(char* destino, int* origen) {
	memcpy(destino, origen, sizeof(int));
	return sizeof(int);
}

int serializar_t_puntero(char* destino, t_puntero* origen) {
	memcpy(destino, origen, sizeof(t_puntero));
	return sizeof(t_puntero);
}

int serializar_stack_elem(char* destino, t_elemento_stack* origen) {
	int desplazamiento = 0;

	desplazamiento += serializar_int(destino + desplazamiento, &(origen->pos));
	desplazamiento += serializar_lista(destino + desplazamiento, origen->argumentos,	sizeof(t_pedido));
//	desplazamiento += serializar_dictionary(destino + desplazamiento, origen->identificadores, sizeof(t_pedido));
	desplazamiento += serializar_t_puntero(destino + desplazamiento, &(origen->posRetorno));
	desplazamiento += serializar_pedido(destino + desplazamiento, &(origen->valRetorno));

	return desplazamiento;
}

int serializar_stack(char* destino, t_stack* origen) {
	int i;
	int desplazamiento = 1;

	// Cantidad de items
	destino[0] = stack_tamanio(origen);

	for (i = 0; i < destino[0]; i++) {
		desplazamiento += serializar_stack_elem(destino + desplazamiento, list_get(origen, i));
	}
	return desplazamiento;
}

void* serializar_PCB(t_PCB* pcb, int sock, int codigoAccion) {
	char* pcbSerializado;
	int desplazamiento = 0;

	desplazamiento = desplazamiento + serializar_int(pcbSerializado + desplazamiento, &(pcb->PID));

	desplazamiento = desplazamiento + serializar_int(pcbSerializado + desplazamiento, &(pcb->contadorPrograma));

	desplazamiento = desplazamiento + serializar_int(pcbSerializado + desplazamiento, &(pcb->cantidadPaginas));

	desplazamiento = desplazamiento + serializar_lista(pcbSerializado + desplazamiento, pcb->indiceCodigo, sizeof(t_sentencia));

	desplazamiento = desplazamiento + serializar_stack(pcbSerializado + desplazamiento, pcb->stackPointer);


	void* buffer = malloc(desplazamiento + sizeof(codigoAccion));
	memcpy(buffer, &codigoAccion, sizeof(codigoAccion)); //PRIMERO EL CODIGO
	memcpy(buffer + sizeof(codigoAccion), pcbSerializado, sizeof(codigoAccion) + desplazamiento ); // SEGUNDO LA DATA

	send(sock, &pcbSerializado, sizeof(codigoAccion) + desplazamiento, 0);

	return 0;
}


