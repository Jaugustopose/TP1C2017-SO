/*
 * fileSystem.h
 *
 *  Created on: 11/4/2017
 *      Author: utnso
 */

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <commons/config.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <unistd.h>

typedef struct{
	int PUERTO;
	char* PUNTO_MONTAJE;
}configuracion_t;

typedef struct{
	int TAMANIO_BLOQUES;
	int CANTIDAD_BLOQUES;
	char* MAGIC_NUMBER;
}metadata_t;

typedef struct{
	int TAMANIO;
	char **BLOQUES;
}archivo_t;

typedef struct{
	char* Archivos;
	char* Bloques;
	char* Metadata;
	char* Bitmap;

}paths_t;

paths_t paths;
configuracion_t config;
metadata_t metadata;
t_bitarray *bitmap;

/*****************************PROTOTIPO*************/
int validarArchivo(char *path);
int crearArchivo(char *path);
void borrarArchivo(char *path);
char* obtenerDatos(char *path, int offset, int size);
void guardarDatos(char *path, int offset, int size, char* buffer);


#endif /* FILESYSTEM_H_ */


