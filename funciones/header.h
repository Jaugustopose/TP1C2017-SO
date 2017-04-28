/*
 * header.h
 *
 *  Created on: 27/4/2017
 *      Author: utnso
 */

#ifndef HEADER_H_
#define HEADER_H_

typedef enum {

	HeaderHandshake // 1

	//Aca vamos agregando todos los Headers que necesitemos (Los del protocolo de envío de msj también)

} header_t;

char* headerToMSG(header_t header);
char* headerToString(header_t header);

#endif /* HEADER_H_ */
