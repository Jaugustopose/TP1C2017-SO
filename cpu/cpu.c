#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include "cpu.h"



#define MAXBYTESREAD 100

//Provisorio: le puse dos para luego factorizar con la de Kernel :D
int crearSocketDos() {
	return socket(AF_INET, SOCK_STREAM, 0);
}

int conectarSocket(int socket, struct sockaddr_in* dirServidor)
{
	return connect(socket, (struct sockaddr*) &*dirServidor, sizeof(struct sockaddr));
}

void cerrarSocket(int socket)
{
   close(socket);
}

void cargarConfiguracion()
{

	t_config* configCpu = config_create("cpu.cfg");
	config.IP_MEMORIA = config_get_int_value(configCpu, "IP_MEMORIA");
    config.PUERTO_MEMORIA = config_get_string_value(configCpu, "PUERTO_MEMORIA");
	config.PUERTO_KERNEL = config_get_string_value(configCpu, "PUERTO_KERNEL");
}

int main(void){

	struct sockaddr_in dirServidor;
	char buf[MAXBYTESREAD];
	int socket;
	int numeroBytes;

	cargarConfiguracion();

	if((socket = crearSocketDos()) == -1)
	{
		perror("No se creo el socket correctamente");
		exit(1);
	}

	//Configuro Servidor
	dirServidor.sin_family = AF_INET;
	dirServidor.sin_port = htons(atoi(config.PUERTO_KERNEL));
	dirServidor.sin_addr.s_addr =  INADDR_ANY;
	memset(&(dirServidor.sin_zero), '\0', 8);

	if(conectarSocket(socket, &dirServidor) == -1)
	{
		perror("No se pudo conectar");
		exit(1);
	}

	if((numeroBytes =  recv(socket, buf, MAXBYTESREAD -1, 0))  == -1)
	{
		perror("Fallo el recv");
		exit(1);
	}

	//Recibo mensaje e informo
	buf[MAXBYTESREAD] = '\0';
	printf("Recibi: %s", buf);
	cerrarSocket(socket);


	return 0;
}
