/*
 * header.c
 *
 *  Created on: 27/4/2017
 *      Author: utnso
 */

#include "header.h"
#include <commons/string.h>

char* headerToMSG(header_t header)
{
	return string_from_format("%c",header);
}

char* headerToString(header_t header) {
	// Se usa la lista de headers de header.h
	// Se busca el Header y se lo reemplaza por un string
	// Ejemplo: encuentro HeaderHandshake --> reemplazo por "HeaderHandshake"

	switch (header) {
		case HeaderHandshake: return "HeaderHandshake";

		//Aca se van agregando todos los Headers que usemos de header.h
	}
}


