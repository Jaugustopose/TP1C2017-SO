/*
 * deserializador.h
 *
 *  Created on: 21/5/2017
 *      Author: utnso
 */

#ifndef DESERIALIZADOR_H_
#define DESERIALIZADOR_H_

/* Tabla de serializacion:

 	 1 = ARCHIVO
 	 2 =
 	 3 =
 	 4 =
 	 5 =

*/

void* deserializar(int sock) {
	int id;

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

}

/*	TODO: Hacer este refactor para el case 1

 	void* deserializar_archivo(int sock) {
	int tamanio;

	recv(sock, &tamanio, 4, 0); //Recibo el tamaño de lo que me estan enviando.
	void* buffer = malloc(tamanio);
	recv(sock, buffer, tamanio,0); //Recibo lo que viene atras. Por ahora es o un archivo o un texto asi que al ser tod0 del mismo tipo, no pasa nada. Caso contrario habrá que ir separandolo con varios recv.

	return buffer;
}*/

#endif /* DESERIALIZADOR_H_ */
