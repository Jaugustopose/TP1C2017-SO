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
};

t_config* configFs;
struct configuracion config;

#endif /* FILESYSTEM_H_ */


