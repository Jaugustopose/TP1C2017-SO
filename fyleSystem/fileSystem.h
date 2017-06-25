/*
 * fileSystem.h
 *
 *  Created on: 11/4/2017
 *      Author: utnso
 */

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include <commons/config.h>
#include <commons/string.h>
#include <commons/bitarray.h>

#include "estructurasCompartidas.h"
#include "cliente-servidor.h"

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
int sockServ; // Socket de conexion
int sockClie; // Socket de escucha

/*****************************PROTOTIPO*************/
int validarArchivo(char *path);
int crearArchivo(char *path);
int borrarArchivo(char *path);
char* obtenerDatos(char *path, int offset, int size);
int guardarDatos(char *path, int offset, int size, char* buffer);


#endif /* FILESYSTEM_H_ */


