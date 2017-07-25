/*
 * serializador.h
 *
 *  Created on: 21/5/2017
 *      Author: utnso
 */

#ifndef SERIALIZADOR_H_
#define SERIALIZADOR_H_

#include "estructurasCompartidas.h"

typedef struct {
	int32_t id;
	int32_t tamanio;
}t_header;


/*char* serializar(t_header header, char* contenidoDelMensaje) {

	char* buffer = malloc(header.tamanio + sizeof(t_header));
	memcpy(buffer, &header.id, sizeof(int)); //PRIMERO EL ID
	memcpy(buffer + 1, &header.tamanio, sizeof(int)); //SEGUNDO EL TAMAÑO
	memcpy(buffer + 2, contenidoDelMensaje, header.tamanio); // TERCERA LA DATA

	return buffer;
	free(buffer);
}*/


void* serializar(t_header header, void* contenidoDelMensaje);
void* serializarMemoria(int codigoAccion, void* contenidoDelMensaje, int tamanioMensaje);
int char4ToInt(char* chars);
char* intToChar4(int num);
int serializar_pedido(char* destino, t_pedido* origen);
int serializar_pedido_bytes(char* destino, pedidoBytesMemoria_t origen);
int serializar_sentencia(char* destino, t_sentencia* origen);
int serializar_lista(char* destino, t_list* origen, int pesoElemento);
int serializar_int(char* destino, int* origen);
int serializar_t_puntero(char* destino, t_puntero* origen);
int serializar_stack_elem(char* destino, t_elemento_stack* origen);
int serializar_stack(char* destino, t_stack* origen);
int serializar_diccionario(char* diccionarioSerializado, t_dictionary* diccionario, int pesoData);
int bytes_list(t_list* origen, int pesoElemento);
int bytes_diccionario(t_dictionary* dic, int pesoData);
int bytes_elemento_stack(t_elemento_stack* origen);
int bytes_stack(t_stack* origen);
int bytes_PCB(t_PCB* pcb);
int serializar_indice_etiquetas(char* destino, char* origen, int tamanio);
void* serializar_PCB(t_PCB* pcb, int sock, int32_t codigoAccion);

#endif /* SERIALIZADOR_H_ */
