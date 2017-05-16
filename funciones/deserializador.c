/*
 * deserializador.c
 *
 *  Created on: 15/5/2017
 *      Author: utnso
 */

#include <stdlib.h>
#include <sys/socket.h>

void* deserializar(int sock) {

	int id;
	int tamanio;
	recv(sock, &id, 4, 0); //Aca recibo la identidad del mensaje (archivo, texto, programa, etc) y hago lo que necesite
	recv(sock, &tamanio, 4, 0); //Recibo el tamaño de lo que me estan enviando.
	void* buffer = malloc(&tamanio);
	recv(sock, buffer, &tamanio, MSG_WAITALL); //Recibo todo lo que viene atras. Por ahora es o un archivo o un texto asi que al ser todo del mismo tipo, no pasa nada. Caso contrario habrá que ir separandolo con varios recv.

	return buffer;
}
