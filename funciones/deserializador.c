/*
 * deserializador.c
 *
 *  Created on: 15/5/2017
 *      Author: utnso
 */

#include <stdlib.h>
#include <sys/socket.h>
#include "deserializador.h"
#include "estructurasCompartidas.h"

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
	return 0;
}

void* deserializar_archivo(int sock) {
	int tamanio;

	recv(sock, &tamanio, 4, 0); //Recibo el tamaño de lo que me estan enviando.
	void* buffer = malloc(tamanio);
	recv(sock, buffer, tamanio,0); //Recibo lo que viene atras. Por ahora es o un archivo o un texto asi que al ser tod0 del mismo tipo, no pasa nada. Caso contrario habrá que ir separandolo con varios recv.

	return buffer;
}

int deserializar_lista(t_list* destino, char* origen, int pesoElemento) {
	int i;
	int offset = 1; // Se va a guardar en la primera posicion la cantidad de elementos
	char* buffer;

	for (i = 0; i < origen[0]; i++) {
		buffer = malloc(pesoElemento);
		memcpy(buffer, origen + offset, pesoElemento);
		list_add(destino, buffer);
		offset = offset + pesoElemento;
	}
	return offset;
}

int deserializar_pedido(t_pedido* destino, char* origen) {
	memcpy(destino, origen, sizeof(t_pedido));
	return sizeof(t_pedido);
}
int deserializar_sentencia(t_sentencia* destino, char* origen) {
	memcpy(destino, origen, sizeof(t_sentencia));
	return sizeof(t_sentencia);

}

int deserializar_int(int* destino, char* origen) {
	memcpy(destino, origen, sizeof(int));
	return sizeof(int);
}

int deserializar_t_puntero(t_puntero* destino, char* origen) {
	memcpy(destino, origen, sizeof(t_puntero));
	return sizeof(t_puntero);
}
int deserializar_stack_elem(t_elemento_stack* elemStack, char* origen) {
	int desplazamiento = 0;

	elemStack->argumentos=list_create();
	elemStack->identificadores=dictionary_create();

	desplazamiento += deserializar_int(&elemStack->pos, origen + desplazamiento);
	desplazamiento += deserializar_lista(elemStack->argumentos, origen + desplazamiento, sizeof(t_pedido));
	desplazamiento += deserializar_diccionario(elemStack->identificadores, origen + desplazamiento, sizeof(t_pedido));
	desplazamiento += deserializar_t_puntero(&elemStack->posRetorno, origen + desplazamiento);
	desplazamiento += deserializar_pedido(&(elemStack->valRetorno), origen + desplazamiento);

	return desplazamiento;
}

int deserializar_stack(t_stack* destino, char* origen) {

	t_elemento_stack* elemen;
	int i;
	int desplazamiento = 1;

	for (i = 0; i < origen[0]; i++) {

		//Tamaño del item como puntero en si
		elemen = malloc(sizeof(t_elemento_stack));
		desplazamiento += deserializar_stack_elem(elemen, origen + desplazamiento);
		stack_push(destino, elemen);
	}

	return desplazamiento;
}

int deserializar_diccionario(t_dictionary* destino, char* origen, int pesoData){

	int i;
	int offset = 1;
	char* key;
	void* value;

	for (i = 0; i < origen[0]; i++){

		int len = origen[offset++];

		key = malloc(len+1);
		memcpy(key, origen+offset, len);
		key[len]='\0';

		offset = offset + len;

		value = malloc(pesoData);
		memcpy(value, origen+offset ,pesoData);
		offset = offset + pesoData;

		dictionary_put(destino,key,value);

		free(key);
		free(value);
	}
	return offset;
}

int deserializar_indice_etiquetas(char* destino, char* origen, int tamanio)
{
	memcpy(destino, origen, tamanio);
	return tamanio;
}

void* deserializar_PCB(t_PCB* pcbUlt, char* pcbSerializado){
//	int tamanio;

	//recv(sock, &tamanio, 4, 0); //Recibo el tamaño de lo que me estan enviando.
	//void* pcbSerializado = malloc(tamanio);
	//char* pcbSerializado = malloc(sizeof(buffer));
	//recv(sock, pcbSerializado, tamanio,0); //Recibo lo que viene atras.

	int desplazamiento = 0;

	pcbUlt->indiceCodigo = list_create();
	//pcbUlt->indiceEtiquetas = dictionary_create();
	pcbUlt->stackPointer = stack_crear();

	desplazamiento = desplazamiento + deserializar_int(&(pcbUlt->PID), pcbSerializado + desplazamiento);

	desplazamiento = desplazamiento + deserializar_int(&(pcbUlt->contadorPrograma), pcbSerializado + desplazamiento);

	desplazamiento = desplazamiento + deserializar_int(&(pcbUlt->cantidadPaginas), pcbSerializado + desplazamiento);

	desplazamiento = desplazamiento + deserializar_lista(pcbUlt->indiceCodigo, pcbSerializado + desplazamiento, sizeof(t_sentencia));

//	desplazamiento = desplazamiento + deserializar_diccionario(pcbUlt->indiceEtiquetas, pcbSerializado + desplazamiento, sizeof(int));

	desplazamiento = desplazamiento + deserializar_int(&(pcbUlt->etiquetasSize), pcbSerializado + desplazamiento);

	pcbUlt->indiceEtiquetas = malloc(pcbUlt->etiquetasSize);

	desplazamiento = desplazamiento + deserializar_indice_etiquetas(pcbUlt->indiceEtiquetas, pcbSerializado + desplazamiento, pcbUlt->etiquetasSize);

	desplazamiento = desplazamiento + deserializar_stack(pcbUlt->stackPointer, pcbSerializado + desplazamiento);


	return 0;
}

