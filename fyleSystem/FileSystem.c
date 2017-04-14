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

int conectarSocket(int socket, struct sockaddr_in* dirServidor)
{
 return connect(socket, (struct sockaddr*) &*dirServidor, sizeof(struct sockaddr));
}

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

	if (config_has_property(configFs, "IP_KERNEL"))
			config.IP_KERNEL = config_get_string_value(configFs,"IP_KERNEL");
	printf("config.IP_KERNEL: %s\n", config.IP_KERNEL);

	if (config_has_property(configFs, "PUERTO_KERNEL"))
			config.PUERTO_KERNEL = config_get_int_value(configFs,"PUERTO_KERNEL");
	printf("config.PUERTO_KERNEL: %d\n", config.PUERTO_KERNEL);

	if (config_has_property(configFs, "IP_FS"))
			config.IP_FS = config_get_string_value(configFs,"IP_FS");
	printf("config.IP_FS: %s\n", config.IP_FS);

	if (config_has_property(configFs, "PUERTO_FS"))
			config.PUERTO_FS = config_get_int_value(configFs,"PUERTO_FS");
	printf("config.PUERTO_FS: %d\n", config.PUERTO_FS);

}

// Programa Principal
int main(void) {
	//printf("Dentro del main\n");

	char* buffer = malloc(5);

	cargarConfiguracion();//Cargo configuracion

    //Creo Cliente
	struct sockaddr_in direccionServidor;//Estructura con la direccion del servidor
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(config.IP_KERNEL);//IP a la que se conecta
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(config.PUERTO_KERNEL);//Puerto al que se conecta

	memset(&(direccionServidor.sin_zero), '\0', 8);

	int cliente = socket(AF_INET, SOCK_STREAM, 0);//Pido un Socket
	printf("cliente: %d\n", cliente);

	if(conectarSocket(cliente, &direccionServidor) == -1)
	 {
	  perror("No se pudo conectar");
	  exit(1);
	 }

	//Recibo mensajes y muestro en pantalla
	while (1) {
		int bytesRecibidos = recv(cliente, buffer, 1000, 0);
		if (bytesRecibidos <= 0) {
			perror("El socket se desconecto\n");
			return 1;
		}

		buffer[bytesRecibidos] = '\0';
		printf("Me llegaron %d bytes --> %s\n", bytesRecibidos, buffer);
	}

	free(buffer);
/*
	//Envio mensajes
	while (1) {
		char mensaje[1000];
		scanf("%s", mensaje);

		send(cliente, mensaje, strlen(mensaje), 0);
	}
*/
	close(cliente);

	return 0;
}
