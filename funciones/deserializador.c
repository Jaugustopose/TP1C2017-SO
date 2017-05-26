/*
 * deserializador.c
 *
 *  Created on: 15/5/2017
 *      Author: utnso
 */

#include <stdlib.h>
#include <sys/socket.h>
#include "deserializador.h"

/* Tabla de serializacion:

 	 1 = ARCHIVO
 	 2 = PCB
 	 3 =
 	 4 =
 	 5 =

*/

void* deserializar(int sock) {
	int id;

	recv(sock, &id, 4, 0); //Aca recibo la identidad del mensaje (archivo, texto, programa, etc)
	switch(id) {

	case 1: deserializar_archivo(sock);
			break;

	}
}

void* deserializar_archivo(int sock) {
	int tamanio;

	recv(sock, &tamanio, 4, 0); //Recibo el tamaño de lo que me estan enviando.
	void* buffer = malloc(tamanio);
	recv(sock, buffer, tamanio,0); //Recibo lo que viene atras. Por ahora es o un archivo o un texto asi que al ser tod0 del mismo tipo, no pasa nada. Caso contrario habrá que ir separandolo con varios recv.

	return buffer;
}


int deserializar_lista(t_list* destino, char* fuente, int pesoElemento) {
	int i;
	int offset = 1; // Se va a guardar en la primera posicion la cantidad de elementos
	char* buffer;

	for (i = 0; i < fuente[0]; i++) {
		buffer = malloc(pesoElemento);
		memcpy(buffer, fuente + offset, pesoElemento);
		list_add(destino, buffer);
		offset = offset + pesoElemento;
	}
	return offset;
}

//int deserializar_stack_item(t_stack_elemento* destino, char* fuente) {
//	int offset = 0;
//	destino->argumentos=list_create();
//	destino->identificadores=dictionary_create();
//	offset += deserializar_int(&destino->posicion, fuente + offset);
//	offset += deserializar_list(destino->argumentos, fuente + offset, sizeof(t_pedido));
//	offset += deserializar_dictionary(destino->identificadores, fuente + offset, sizeof(t_pedido));
//	offset += deserializar_t_puntero(&destino->posicionRetorno, fuente + offset);
//	offset += deserializar_pedido(&(destino->valorRetorno), fuente + offset);
//	return offset;
//}

void* deserializar_PCB(t_PCB* pcbNuevo, int sock) {
	int tamanio;

	recv(sock, &tamanio, 4, 0); //Recibo el tamaño de lo que me estan enviando.
	void* pcbSerializado = malloc(tamanio);
	recv(sock, pcbSerializado, tamanio,0); //Recibo lo que viene atras.

	int desplazamiento = 0;

	pcbNuevo->indiceCodigo = list_create();
	pcbNuevo->stackPointer = stack_crear();


	memcpy(&(pcbNuevo->PID), pcbSerializado + desplazamiento, sizeof(int));
	desplazamiento = desplazamiento + sizeof(int);
	printf("PID: %d\n", pcbNuevo->PID);

	memcpy(&(pcbNuevo->contadorPrograma), pcbSerializado + desplazamiento, sizeof(int));
	desplazamiento = desplazamiento + sizeof(int);
	printf("PC: %d\n", pcbNuevo->contadorPrograma);

	memcpy(&(pcbNuevo->cantidadPaginas), pcbSerializado + desplazamiento, sizeof(int));
	desplazamiento = desplazamiento + sizeof(int);
	printf("CantPag: %d\n", pcbNuevo->cantidadPaginas);
	//desplazamiento = desplazamiento + deserializar_lista(pcbNuevo->indiceCodigo, pcbSerializado + desplazamiento, sizeof(t_sentencia));

	//desplazamiento = desplazamiento + deserializar_stack(pcbNuevo->stackPointer, pcbSerializado + desplazamiento);


	return 0;
}
