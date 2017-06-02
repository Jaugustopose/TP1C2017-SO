/*
 * fileSystem.h
 *
 *  Created on: 11/4/2017
 *      Author: utnso
 */

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <commons/config.h>
#include <unistd.h>

typedef struct configuracion {
	char* IP_KERNEL;
	int PUERTO_KERNEL;
	char* IP_FS;
	int PUERTO_FS;
};

t_config* configFs;
struct configuracion config;

/*****************************PROTOTIPO*************/
void validarArchivo(char path);
void crearArchivo(char path);
void borrarArchivo(char path);
char* obtenerDatos(char path, int offset, int size);
void guardarDatos(char path, int offset, int size, char* buffer);


#endif /* FILESYSTEM_H_ */


