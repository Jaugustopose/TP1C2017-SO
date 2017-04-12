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
			config.IP_KERNEL = config_get_string_value(configFs,"IP_KERNEL");
	printf("config.IP_KERNEL: %s\n", config.IP_KERNEL);
	if (config_has_property(configFs, "PUERTO_KERNEL"))
			config.PUERTO_KERNEL = config_get_int_value(configFs,"PUERTO_KERNEL");
	printf("config.PUERTO_KERNEL: %d\n", config.PUERTO_KERNEL);
}

// Programa Principal
int main(void) {
	printf("Dentro del main\n");

	cargarConfiguracion();//Cargo configuracion
    //Creo Cliente
	struct sockaddr_in direccionServidor;//Estructura con la direccion del servidor
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(config.IP_KERNEL);//IP a la que se conecta
	direccionServidor.sin_port = htons(config.PUERTO_KERNEL);//Puerto al que se conecta

	int cliente = socket(AF_INET, SOCK_STREAM, 0);//Pido un Socket

	//Me conecto al Servidor
	if (connect(cliente, (void*) &direccionServidor, sizeof(direccionServidor)) != 0) {
		perror("Error al conectar");
		return 1;
	}

	return 0;
}
