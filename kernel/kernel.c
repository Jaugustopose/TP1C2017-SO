/*
 * kernel.c
 *
 *  Created on: 2/4/2017
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "kernel.h"
#include <commons/string.h>
#define BACKLOG 10

int crearSocket() {
	return socket(AF_INET, SOCK_STREAM, 0);
}

int reusarSocket(int sockServ, int yes) {
	return setsockopt(sockServ, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
}

int bindearSocket(int sockServ, const struct sockaddr_in* mi_addr) {
	return bind(sockServ, (struct sockaddr*) &*mi_addr, sizeof(struct sockaddr));
}

int listenearSocket(int sockServ) {
	return listen(sockServ, BACKLOG);
}

void cargarConfiguracion()
{
	char* pat = string_new();
	char cwd[1024]; // Variable donde voy a guardar el path absoluto hasta el /Debug
	string_append(&pat,getcwd(cwd,sizeof(cwd)));
	string_append(&pat,"/kernel.cfg");
	t_config* configKernel = config_create(pat);
	printf("El directorio sobre el que se esta trabajando es %s\n", pat);
	free(pat);
	if (config_has_property(configKernel, "IP_MEMORIA"))
			config.IP_MEMORIA = config_get_string_value(configKernel,"IP_MEMORIA");
	if (config_has_property(configKernel, "IP_FS"))
				config.IP_FS = config_get_string_value(configKernel,"IP_FS");
	if (config_has_property(configKernel, "PUERTO_KERNEL"))
					config.PUERTO_KERNEL = config_get_int_value(configKernel,"PUERTO_KERNEL");
	if (config_has_property(configKernel, "PUERTO_MEMORIA"))
					config.PUERTO_MEMORIA = config_get_int_value(configKernel,"PUERTO_MEMORIA");
	if (config_has_property(configKernel, "PUERTO_CPU"))
					config.PUERTO_CPU = config_get_int_value(configKernel,"PUERTO_CPU");
	if (config_has_property(configKernel, "PUERTO_FS"))
					config.PUERTO_FS = config_get_int_value(configKernel,"PUERTO_FS");
	if (config_has_property(configKernel, "PUERTO_CONSOLA"))
					config.PUERTO_CONSOLA = config_get_int_value(configKernel,"PUERTO_CONSOLA");


    /*config.PUERTO_MEMORIA = config_get_int_value(configKernel, "PUERTO_MEMORIA");
	config.PUERTO_KERNEL = config_get_int_value(configKernel, "PUERTO_KERNEL");
	config.PUERTO_FS = config_get_int_value(configKernel, "PUERTO_FS");
	config.PUERTO_CPU = config_get_int_value(configKernel, "PUERTO_CPU");
	config.PUERTO_CONSOLA = config_get_int_value(configKernel, "PUERTO_CONSOLA");
	config.IP_MEMORIA = config_get_string_value(configKernel, "IP_MEMORIA");
	config.IP_FS = config_get_string_value(configKernel, "IP_FS");
	*/
}

int main(void)
{

	//VARIABLES

	fd_set master; // Conjunto maestro de file descriptor (Donde me voy a ir guardando todos los socket nuevos)
	fd_set read_fds; // Conjunto temporal de file descriptors para pasarle al select()
	struct sockaddr_in mi_addr; // Información sobre mi dirección
	struct sockaddr_in clie_addr; // Información sobre la dirección del cliente
	int sockServ; // Socket de nueva conexion aceptada
	int sockClie; // Socket a la escucha
	int maxSock; // Numero del ultimo socket creado (maximo file descriptor)
	int yes=1;
	int cantBytes; // La cantidad de bytes. Lo voy a usar para saber cuantos bytes me mandaron.
	int addrlen; // El tamaño de la direccion del cliente
	int i, j; // Variables para recorrer los sockets (mandar mensajes o detectar datos con el select)
	//char buff[100]; // Buffer para datos que me manda el cliente
	FD_ZERO(&master); // borra los conjuntos maestro y temporal por si tienen basura adentro (capaz no hacen falta pero por las dudas)
	FD_ZERO(&read_fds);


	cargarConfiguracion();


	//Creo el socket
	if ((sockServ = crearSocket()) == -1) {
	puts("Error al crear socket");
	exit(1);
	}

	//Lo hago reutilizable
	if (reusarSocket(sockServ, yes) == -1) {
	printf("Error al tratar de reusar socket");
	exit(1);
	}

	//Bindear socket a cliente
	mi_addr.sin_family = AF_INET; // Ordenación de bytes de la máquina
	mi_addr.sin_port = htons(config.PUERTO_KERNEL); // short, Ordenación de bytes de la red
	mi_addr.sin_addr.s_addr = INADDR_ANY; // Rellenar con mi dirección IP
	memset(&(mi_addr.sin_zero), '\0', 8); // Poner ceros para rellenar el resto de la estructura
	if (bindearSocket(sockServ, &mi_addr)== -1) {
	printf("Error al tratar de bindear");
	exit(1);
	}

	//Dejar socket escuchando
	if (listenearSocket(sockServ) == -1) {
	printf("Error al tratar de dejar el socket listeneando");
	exit(1);
	}
	printf("Estoy escuchando\n");

	// añadir listener al conjunto maestro
	FD_SET(sockServ, &master);

	// Mantener actualizado cual es el maxSock
	maxSock = sockServ;

	// bucle principal
	for(;;) {
	read_fds = master; // Me paso lo que tenga en el master al temporal.
	if (select(maxSock+1, &read_fds, NULL, NULL, NULL) == -1) { //Compruebo los sockets al mismo tiempo. Los NULL son para los writefds, exceptfds y el timeval.
		perror("select");
		exit(1);
		}

	// explorar conexiones existentes en busca de datos que leer
	for(i = 0; i <= maxSock; i++) {
		if (FD_ISSET(i, &read_fds)) { // Me fijo si tengo datos listos para leer
			if (i == sockServ) { //si entro en este "if", significa que tengo datos.
				// gestionar nuevas conexiones
				addrlen = sizeof(clie_addr);
				if ((sockClie = accept(sockServ, (struct sockaddr*)&clie_addr, &addrlen)) == -1){
					perror("accept");
							} else {
								FD_SET(sockClie, &master); // añadir al conjunto maestro
									if (sockClie > maxSock) { // actualizar el máximo
										maxSock = sockClie;
									}
								printf("Server: nueva conexion de %s en socket %d\n", inet_ntoa(clie_addr.sin_addr), sockClie);
								}
			} else 	{
				// gestionar datos de un cliente
							char* buff = malloc(1000);

							if ((cantBytes = recv(i, buff, 1000, 0)) <= 0) {

								// error o conexión cerrada por el cliente
								if (cantBytes == 0) {

									// conexión cerrada
									printf("Server: socket %d termino la conexion\n", i);
								} else {
									perror("recv");
								}
								close(i); // Si se perdio la conexion, nos vimos en disney
								FD_CLR(i, &master); // Eliminar del conjunto maestro
							}
								else{ // tenemos datos de algún cliente
									printf("He recibido %d bytes de contenido: %s\n", cantBytes, buff);

									for(j = 0; j <= maxSock; j++) { // Enviar a todos
										if (FD_ISSET(j, &master)) { // Me fijo si esta en el master
											// excepto al Servidor y al mismo hermoso que manda el mensaje
											if (j != sockServ && j != i) {
												if (send(j, buff, cantBytes, 0) == -1) {
													perror("send");
													}
												}
											}
										}
									free(buff);
									}
					}
		}

		/* PARA EL FUTURO PROTOCOLO DE ENVIO DE MENSAJES

	 	uint32_t tamanioPaquete;
		recv(sockClie, &tamanioPaquete, 4, 0);

		char* buff = malloc(tamanioPaquete);
		recv(sockClie, buff, tamanioPaquete, MSG_WAITALL);
		 */
	}
	}
	return 0;

}
