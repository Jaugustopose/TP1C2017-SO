/*
 * FileSystem.c
 *
 *  Created on: 7/4/2017
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include "fileSystem.h"

void cargarConfiguracion()
{
	char* pat = string_new();
	char cwd[1024]; // Variable donde voy a guardar el path absoluto hasta el /Debug
	string_append(&pat,getcwd(cwd,sizeof(cwd)));
	string_append(&pat,"/FileSystem.cfg");
	t_config* configFs = config_create(pat);

	printf("El directorio sobre el que se esta trabajando es %s\n", pat);
	free(pat);
	printf("despues del free\n");
	if (config_has_property(configFs, "IP_MEMORIA"))
			config.ipKernel = config_get_string_value(configFs,"IP_KERNEL");
	printf("config.ipKernel: %s\n", config.ipKernel);
	if (config_has_property(configFs, "PUERTO_KERNEL"))
			config.puertoKernel = config_get_int_value(configFs,"PUERTO_KERNEL");
	printf("config.puertoKernel: %d\n", config.ipKernel);
}

// Programa Principal
int main(void) {
	printf("Dentro del main\n");

	cargarConfiguracion();

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(config.ipKernel);
	direccionServidor.sin_port = htons(config.puertoKernel);

	int cliente = socket(AF_INET, SOCK_STREAM, 0);

	if (connect(cliente, (void*) &direccionServidor, sizeof(direccionServidor)) != 0) {
		perror("Error al conectar");
		return 1;
	}

	return 0;
}
