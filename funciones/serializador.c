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


void* serializar(t_header cabeza, void* contenidoDelMensaje) {

	void* buffer = malloc(sizeof(t_header) + cabeza.tamanio);
	memcpy(buffer,&cabeza.id, sizeof(int32_t)); //PRIMERO EL ID
	memcpy(buffer + sizeof(cabeza.id),&cabeza.tamanio, sizeof(int32_t)); //SEGUNDO EL TAMAÑO
	memcpy(buffer + sizeof(t_header), contenidoDelMensaje, cabeza.tamanio); // TERCERA LA DATA

	return buffer;
}

void* serializarMemoria(int codigoAccion, void* contenidoDelMensaje, int tamanioMensaje) {
	printf("tamanio mensaje: %d\n", tamanioMensaje);
	void* buffer = malloc(tamanioMensaje + sizeof(codigoAccion));
	memcpy(buffer, &codigoAccion, sizeof(codigoAccion)); //PRIMERO EL CODIGO
	memcpy(buffer + sizeof(codigoAccion), contenidoDelMensaje, tamanioMensaje); // SEGUNDO LA DATA

	return buffer;
}

int char4ToInt(char* chars){
	int a;
	deserializar_int(&a,chars);
	return a;
}

char* intToChar4(int num){

	char* serial = malloc(sizeof(int));
	serializar_int(serial,&num);
	return serial;
}



int serializar_pedido(char* destino, t_pedido* origen) {

	memcpy(destino, &origen, sizeof(t_pedido));
	return (int)sizeof(t_pedido);
}

int serializar_pedido_bytes(char* destino, pedidoBytesMemoria_t origen) {

	memcpy(&destino, &origen, sizeof(pedidoBytesMemoria_t));
	return (int)sizeof(pedidoBytesMemoria_t);
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
 	desplazamiento += serializar_diccionario(destino + desplazamiento, origen->identificadores, sizeof(t_pedido));
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

int serializar_diccionario(char* diccionarioSerializado, t_dictionary* diccionario, int pesoData){
	int indiceTabla;
	int tamanioReal = 0;
	int offset = 1;

	diccionarioSerializado[0] = diccionario->table_max_size;

	for (indiceTabla = 0; indiceTabla < diccionarioSerializado[0]; indiceTabla++) {

		t_hash_element *elem = diccionario->elements[indiceTabla];

		while (elem != NULL) {

			tamanioReal++;

			diccionarioSerializado[offset++] = strlen(elem->key);

			memcpy(diccionarioSerializado + offset, elem->key, strlen(elem->key));
			offset = offset + strlen(elem->key);

			memcpy(diccionarioSerializado + offset, elem->data, pesoData);
			offset = offset + pesoData;

			//Siguiente elemento
			elem = elem->next;
		}
	}

	//El verdadero tamanio
	diccionarioSerializado[0] = tamanioReal;

	return offset;
}


int bytes_list(t_list* origen, int pesoElemento){

	return (int)(1 + (list_size(origen)*pesoElemento));
}

int bytes_diccionario(t_dictionary* dic, int pesoData){

	int indiceTabla = 1;
	int bytes = 1;

	for (indiceTabla = 0; indiceTabla < dic->table_max_size; indiceTabla++) {

		t_hash_element *element = dic->elements[indiceTabla];

		while (element != NULL) {
			bytes++; // La cantidad de caracteres de la key del elemento
			bytes = bytes + strlen(element->key);
			bytes = bytes + pesoData;

			element = element->next;
		}
	}
	return bytes;
}

int bytes_elemento_stack(t_elemento_stack* origen) {
	return sizeof(int) + bytes_list(origen->argumentos, sizeof(t_pedido))
			+ bytes_diccionario(origen->identificadores, sizeof(t_pedido))
			+ sizeof(t_puntero) + sizeof(t_pedido);
}

int bytes_stack(t_stack* origen) {
	int i = 1;
	int bytes = 1;

	for (i = 0; i < stack_tamanio(origen); i++) {

		bytes = bytes + bytes_elemento_stack(list_get(origen, i));
	}
	return bytes;
}


int bytes_PCB(t_PCB* pcb) {
	return (int)((sizeof(int) * 3)
			+ bytes_stack(pcb->stackPointer)
			+ bytes_list(pcb->indiceCodigo, sizeof(t_sentencia))
			+ bytes_diccionario(pcb->indiceEtiquetas, sizeof(int))
			);
}

void* serializar_PCB(t_PCB* pcb, int sock, int32_t codigoAccion) {

	int tamanioEnBytes = bytes_PCB(pcb);
	void* pcbSerializado = malloc(tamanioEnBytes);
	int desplazamiento = 0;

	desplazamiento = desplazamiento + serializar_int(pcbSerializado + desplazamiento, &(pcb->PID));

	desplazamiento = desplazamiento + serializar_int(pcbSerializado + desplazamiento, &(pcb->contadorPrograma));

	desplazamiento = desplazamiento + serializar_int(pcbSerializado + desplazamiento, &(pcb->cantidadPaginas));

	desplazamiento = desplazamiento + serializar_lista(pcbSerializado + desplazamiento, pcb->indiceCodigo, sizeof(t_sentencia));

	desplazamiento = desplazamiento + serializar_diccionario(pcbSerializado + desplazamiento, pcb->indiceEtiquetas, sizeof(int));

	desplazamiento = desplazamiento + serializar_stack(pcbSerializado + desplazamiento, pcb->stackPointer);


	void* buffer = malloc(desplazamiento + sizeof(codigoAccion) + sizeof(tamanioEnBytes));
	memcpy(buffer, &codigoAccion, sizeof(codigoAccion)); //PRIMERO EL CODIGO
	memcpy(buffer + sizeof(tamanioEnBytes), &tamanioEnBytes, sizeof(tamanioEnBytes)); //SEGUNDO EL TAMAÑO DE LA DATA
	memcpy(buffer + sizeof(tamanioEnBytes) + sizeof(codigoAccion), pcbSerializado, sizeof(codigoAccion)+ sizeof(tamanioEnBytes) + desplazamiento); // TERCERO LA DATA

	int bytesEnviados = send(sock, buffer, sizeof(codigoAccion) + desplazamiento + sizeof(tamanioEnBytes), 0);

	return pcbSerializado;
}


