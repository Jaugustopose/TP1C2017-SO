/*
 * deserializador.h
 *
 *  Created on: 21/5/2017
 *      Author: utnso
 */

#ifndef DESERIALIZADOR_H_
#define DESERIALIZADOR_H_
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include "estructurasCompartidas.h"


/* Tabla de serializacion:

 	 1 = ARCHIVO
 	 2 =
 	 3 =
 	 4 =
 	 5 =

*/



/*
	recv(sock, &id, 4, 0); //Aca recibo la identidad del mensaje (archivo, texto, programa, etc)
	switch(id) {

		case 1:  {
				int tamanio;

				recv(sock, &tamanio, 4, 0); //Recibo el tamaño de lo que me estan enviando.
				char* buffer = malloc(tamanio);
				recv(sock, buffer, tamanio,0); //Recibo lo que viene atras. Por ahora es o un archivo o un texto asi que al ser tod0 del mismo tipo, no pasa nada. Caso contrario habrá que ir separandolo con varios recv.

				return buffer;
				break;
		}

	}

}*/

/*	TODO: Hacer este refactor para el case 1

 	void* deserializar_archivo(int sock) {
	int tamanio;

	recv(sock, &tamanio, 4, 0); //Recibo el tamaño de lo que me estan enviando.
	void* buffer = malloc(tamanio);
	recv(sock, buffer, tamanio,0); //Recibo lo que viene atras. Por ahora es o un archivo o un texto asi que al ser tod0 del mismo tipo, no pasa nada. Caso contrario habrá que ir separandolo con varios recv.

	return buffer;
}*/
void* deserializar(int sock);
void* deserializar_archivo(int sock);
int deserializar_lista(t_list* destino, char* origen, int pesoElemento);
int deserializar_pedido(t_pedido* destino, char* origen);
int deserializar_sentencia(t_sentencia* destino, char* origen);
int deserializar_int(int* destino, char* origen);
int deserializar_t_puntero(t_puntero* destino, char* origen);
int deserializar_stack_elem(t_elemento_stack* elemStack, char* origen);
int deserializar_stack(t_stack* destino, char* origen);
int deserializar_diccionario(t_dictionary* destino, char* origen, int pesoData);
int deserializar_indice_etiquetas(char* destino, char* origen, int tamanio);
void* deserializar_PCB(t_PCB* pcbUlt, char* pcbSerializado);

#endif /* DESERIALIZADOR_H_ */
