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
#include "cliente-servidor.h"

void cargarConfiguracion() {
	char* pat = string_new();
	char cwd[1024]; // Variable donde voy a guardar el path absoluto hasta el /Debug
	string_append(&pat, getcwd(cwd, sizeof(cwd)));
	string_append(&pat, "/Debug/kernel.cfg");
	t_config* configKernel = config_create(pat);
	free(pat);

	if (config_has_property(configKernel, "IP_MEMORIA")) {
		config.IP_MEMORIA = config_get_string_value(configKernel, "IP_MEMORIA");
		printf("IP_MEMORIA: %s\n", config.IP_MEMORIA);
	}
	if (config_has_property(configKernel, "IP_FS")) {
		config.IP_FS = config_get_string_value(configKernel, "IP_FS");
		printf("IP_FS: %s\n", config.IP_FS);
	}
	if (config_has_property(configKernel, "PUERTO_KERNEL")) {
		config.PUERTO_KERNEL = config_get_int_value(configKernel,
				"PUERTO_KERNEL");
		printf("PUERTO_KERNEL: %d\n", config.PUERTO_KERNEL);
	}
	if (config_has_property(configKernel, "PUERTO_MEMORIA")) {
		config.PUERTO_MEMORIA = config_get_int_value(configKernel,
				"PUERTO_MEMORIA");
		printf("PUERTO_MEMORIA: %d\n", config.PUERTO_MEMORIA);
	}
	if (config_has_property(configKernel, "PUERTO_CPU")) {
		config.PUERTO_CPU = config_get_int_value(configKernel, "PUERTO_CPU");
		printf("PUERTO_CPU: %d\n", config.PUERTO_CPU);
	}
	if (config_has_property(configKernel, "PUERTO_FS")) {
		config.PUERTO_FS = config_get_int_value(configKernel, "PUERTO_FS");
		printf("PUERTO_FS: %d\n", config.PUERTO_FS);
	}
	if (config_has_property(configKernel, "PUERTO_CONSOLA")) {
		config.PUERTO_CONSOLA = config_get_int_value(configKernel,
				"PUERTO_CONSOLA");
		printf("PUERTO_CONSOLA: %d\n", config.PUERTO_CONSOLA);
	}
	if (config_has_property(configKernel, "GRADO_MULTIPROG")) {
		config.GRADO_MULTIPROG = config_get_int_value(configKernel,
				"GRADO_MULTIPROG");
		printf("GRADO_MULTIPROG: %d\n\n\n", config.GRADO_MULTIPROG);
	}

	printf("--------------Configuración cargada exitosamente--------------\n\n");
	printf("Seleccione la opción que desee realizar:\n"
			"1) Listado de procesos del sistema\n"
			"2) Finalizar un proceso\n"
			"3) Consultar estado de un proceso\n"
			"4) Detener planificación\n");

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
						recv(sockClie, (int*) buf, sizeof(int), 0);
						identidadCliente = *buf;
						switch (identidadCliente) {

						case (int) 1:
							FD_SET(sockClie, &bolsaConsolas); //agrego una nueva consola a la bolsa de consolas
							printf("Se ha conectado una nueva consola, pero no fue agregada a bolsaConsolas\n");
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

					if ((cantBytes = recv(i, buff, 5, 0)) <= 0) {

						// error o conexión cerrada por el cliente
						if (cantBytes == 0) {

							// conexión cerrada
							printf("Server: socket %d termino la conexion\n",
									i);
						} else {
							perror("recv");
						}

						// Eliminar del conjunto maestro y su respectiva bolsa
						FD_CLR(i, &master);
						if (FD_ISSET(i, &bolsaConsolas)) {
							FD_CLR(i, &bolsaConsolas);
						} else {
							FD_CLR(i, &bolsaCpus);
						}
						close(i); // Si se perdio la conexion, la cierro.

					} else { // tenemos datos de algún cliente
							 //Se manda cantBYtes -1 porque es lo que debe mostrar sin el /0
						printf("He recibido %d bytes de contenido: %.*s\n",
								cantBytes, cantBytes - 1, buff);
						for (j = 0; j <= maxSock; j++) { // Enviar a todos
							if (FD_ISSET(j, &master)) { // Me fijo si esta en el master
								//Hago cosas en función de la bolsa en la que este.

								if (FD_ISSET(j, &bolsaConsolas)) {
									// Aca adentro va todo lo que quiero hacer si el cliente es una Consola
									puts("Hola consolas");
									t_PCB pcb; //Creo el PCB cuando la consola manda codigo
									send(j, buff, cantBytes, 0);

								} else {
									if (FD_ISSET(j, &bolsaCpus)) {
										// Aca adentro va todo lo que quiero hacer si el cliente es un CPU
										puts("Hola cpus");
										send(j, buff, cantBytes, 0);

									}
								}
							}
						}
						free(buff);
					}
				}
			}
		}
	}
	return 0;

}

