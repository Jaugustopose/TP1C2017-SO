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

void* deserializar(int sock);

void* deserializar_archivo(int sock);

#endif /* DESERIALIZADOR_H_ */
