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

void cargarConfiguracion() {
	char* pat = string_new();
	char cwd[1024]; // Variable donde voy a guardar el path absoluto hasta el /Debug
	string_append(&pat, getcwd(cwd, sizeof(cwd)));
	string_append(&pat, "/Debug/kernel.cfg");
	printf("El directorio sobre el que se esta trabajando es %s\n", pat);
	t_config* configKernel = config_create(pat);
	free(pat);

	if (config_has_property(configKernel, "IP_MEMORIA")) {
		config.IP_MEMORIA = config_get_string_value(configKernel, "IP_MEMORIA");
		printf("config.IP_MEMORIA: %s\n", config.IP_MEMORIA);
	}
	if (config_has_property(configKernel, "IP_FS")) {
		config.IP_FS = config_get_string_value(configKernel, "IP_FS");
		printf("config.IP_FS: %s\n", config.IP_FS);
	}
	if (config_has_property(configKernel, "PUERTO_KERNEL")) {
		config.PUERTO_KERNEL = config_get_int_value(configKernel,
				"PUERTO_KERNEL");
		printf("config.PUERTO_KERNEL: %d\n", config.PUERTO_KERNEL);
	}
	if (config_has_property(configKernel, "PUERTO_MEMORIA")) {
		config.PUERTO_MEMORIA = config_get_int_value(configKernel,
				"PUERTO_MEMORIA");
		printf("config.PUERTO_MEMORIA: %d\n", config.PUERTO_MEMORIA);
	}
	if (config_has_property(configKernel, "PUERTO_CPU")) {
		config.PUERTO_CPU = config_get_int_value(configKernel, "PUERTO_CPU");
		printf("config.PUERTO_CPU: %d\n", config.PUERTO_CPU);
	}
	if (config_has_property(configKernel, "PUERTO_FS")) {
		config.PUERTO_FS = config_get_int_value(configKernel, "PUERTO_FS");
		printf("config.PUERTO_FS: %d\n", config.PUERTO_FS);
	}
	if (config_has_property(configKernel, "PUERTO_CONSOLA")) {
		config.PUERTO_CONSOLA = config_get_int_value(configKernel,
				"PUERTO_CONSOLA");
		printf("config.PUERTO_CONSOLA: %d\n", config.PUERTO_CONSOLA);
	}
}

void comprobarSockets(int maxSock, fd_set* read_fds) {
	if (select(maxSock + 1, &read_fds, NULL, NULL, NULL) == -1) { //Compruebo los sockets al mismo tiempo. Los NULL son para los writefds, exceptfds y el timeval.
		perror("select");
		exit(1);
	}
}

int main(void) {

	//VARIABLES

	fd_set master; // Conjunto maestro de file descriptor (Donde me voy a ir guardando todos los socket nuevos)
	fd_set read_fds; // Conjunto temporal de file descriptors para pasarle al select()
	fd_set bolsaConsolas; // Bolsa de consolas
	fd_set bolsaCpus; //Bolsa de bolsaCpus
	struct sockaddr_in direccionServidor; // Información sobre mi dirección
	struct sockaddr_in direccionCliente; // Información sobre la dirección del cliente
	int sockServ; // Socket de nueva conexion aceptada
	int sockClie; // Socket a la escucha
	int maxSock; // Numero del ultimo socket creado (maximo file descriptor)
	int yes = 1;
	int cantBytes; // La cantidad de bytes. Lo voy a usar para saber cuantos bytes me mandaron.
	int addrlen; // El tamaño de la direccion del cliente
	int identidadCliente;
	int i, j; // Variables para recorrer los sockets (mandar mensajes o detectar datos con el select)
	FD_ZERO(&master); // Borro por si tienen basura adentro (capaz no hacen falta pero por las dudas)
	FD_ZERO(&read_fds);
	FD_ZERO(&bolsaConsolas);
	FD_ZERO(&bolsaCpus);

	cargarConfiguracion();

	//Crear socket. Dejar reutilizable. Crear direccion del servidor. Bind. Listen.
	sockServ = crearSocket();
	reusarSocket(sockServ, yes);
	direccionServidor = crearDireccionServidor(config.PUERTO_KERNEL);
	bind_w(sockServ, &direccionServidor);
	listen_w(sockServ);

	// añadir listener al conjunto maestro
	FD_SET(sockServ, &master);

	// Mantener actualizado cual es el maxSock
	maxSock = sockServ;

	// bucle principal
	for (;;) {
		read_fds = master; // Me paso lo que tenga en el master al temporal.
		if (select(maxSock + 1, &read_fds, NULL, NULL, NULL) == -1) { //Compruebo los sockets al mismo tiempo. Los NULL son para los writefds, exceptfds y el timeval.
			perror("select");
			exit(1);
		};
		// explorar conexiones existentes en busca de datos que leer
		for (i = 0; i <= maxSock; i++) {
			if (FD_ISSET(i, &read_fds)) { // Me fijo si tengo datos listos para leer
				if (i == sockServ) { //si entro en este "if", significa que tengo datos.

					// gestionar nuevas conexiones
					addrlen = sizeof(direccionCliente);
					if ((sockClie = accept(sockServ,
							(struct sockaddr*) &direccionCliente, &addrlen))
							== -1) {
						perror("accept");
					} else {
						printf("Server: nueva conexion de %s en socket %d\n",
								inet_ntoa(direccionCliente.sin_addr), sockClie);

						FD_SET(sockClie, &master); // añadir al conjunto maestro

						//Recibo identidad y coloco en la bolsa correspondiente

						int* buf = malloc(sizeof(int));
						recv(sockClie, (int*) buf, sizeof(int), 0); //HASTA LO ESTOY CASTEANDO DOBLEMENTE ACA!!!!!
						identidadCliente = *buf;
						switch (identidadCliente) {

						case (int) 1:
							FD_SET(sockClie, &bolsaConsolas); //agrego una nueva consola a la bolsa de consolas
							break;
						case (int) 2:
							FD_SET(sockClie, &bolsaCpus); //agrego un nuevo cpu a la bolsa de cpus
							break;
						}
						if (sockClie > maxSock) { // actualizar el máximo
							maxSock = sockClie;
						}

					}
				} else {
					// gestionar datos de un cliente
					char* buff = malloc(1000);

					if ((cantBytes = recv(i, buff, 1000, 0)) <= 0) {

						// error o conexión cerrada por el cliente
						if (cantBytes == 0) {

							// conexión cerrada
							printf("Server: socket %d termino la conexion\n",
									i);
						} else {
							perror("recv");
						}
						close(i); // Si se perdio la conexion, nos vimos en disney
						FD_CLR(i, &master); // Eliminar del conjunto maestro
					} else { // tenemos datos de algún cliente
							 //Se manda cantBYtes -1 porque es lo que debe mostrar sin el /0
						printf("He recibido %d bytes de contenido: %.*s\n",
								cantBytes, cantBytes - 1, buff);

						for (j = 0; j <= maxSock; j++) { // Enviar a todos
							if (FD_ISSET(j, &master)) { // Me fijo si esta en el master
								//Hago cosas en función de la bolsa en la que este.

								if (FD_ISSET(j, &bolsaConsolas)) {
									puts("Hola consolas");
									send(j, buff, cantBytes, 0);

								} else {
									if (FD_ISSET(j, &bolsaCpus)) {
										puts("Hola cpus");
										send(j, buff, cantBytes, 0);

									}
									/*if (j != sockServ && j!=i) {
									 if (send(j, buff, cantBytes, 0) == -1) {
									 perror("send");
									 }*/
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

