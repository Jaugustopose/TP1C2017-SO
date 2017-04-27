/*
 * cliente-servidor.c
 *
 *  Created on: 27/4/2017
 *      Author: utnso
 */

#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "cliente-servidor.h"
#include <commons/string.h>

int socket_ws() {
	int sock;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		puts("Error al crear socket");
		exit(1);
	}
	return sock;
}

void send_w(int cliente, char* msg, int msgSize) {
	send(cliente, msg, msgSize, 0);
}

struct sockaddr_in crearDireccionParaCliente(unsigned short PORT, char* IP) {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(IP);
	direccionServidor.sin_port = htons(PORT);
	return direccionServidor;
}

void connect_w(int cliente, struct sockaddr_in* direccionServidor) {
	if (connect(cliente, (void*) direccionServidor, sizeof(*direccionServidor))
			!= 0) {
		perror("No se pudo conectar");
		exit(1);
	}
}

void permitirReutilizacion(int servidor, int activado) {
	if (setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, activado, sizeof(activado)) == -1) {
			printf("Error al tratar de reusar socket");
			exit(1);
		}
}

char* recv_nowait_ws(int cliente, int msgSize) //Recibir sin MSG_WAITALL
{
	char* buffer = malloc(msgSize);
	int bytesRecibidos = recv(cliente, buffer, msgSize, 0);
	if (bytesRecibidos <= 0) {
		perror("recv devolvio un numero menor que cero");
		exit(1);
	}
	//buffer[msgSize-1]='\0';
	return buffer;
}

void listen_ws(int servidor) {
	if (listen(servidor, 100) == -1) {
			printf("Error al tratar de dejar el socket escuchando");
			exit(1);
		}
	printf("Estoy escuchando...\n");
}

struct sockaddr_in crearDireccionParaServidor(unsigned short PORT) {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(PORT);
	return direccionServidor;
}

void bind_ws(int servidor, struct sockaddr_in* direccionServidor) {
	if (bind(servidor, (void*) direccionServidor, sizeof(*direccionServidor))
			!= 0) {
		perror("Fallo el bind");
		exit(1);
	}
}

void configurarServidor(unsigned short PORT) {
	// Definimos la direccion, creamos el socket, bindeamos y ponemos a escuchar nuevas conexiones
	direccion = crearDireccionParaServidor(PORT);
	socketNuevasConexiones = socket_ws();
	permitirReutilizacion(socketNuevasConexiones, activado);
	bind_ws(socketNuevasConexiones, &direccion);
	listen_ws(socketNuevasConexiones);
}

int agregarCliente(t_cliente cliente) {
	// Agregamos un cliente en la primera posicion libre del array de clientes
	int i = 0;
	for (i = 0; i < MAXCLIENTS; i++) {
		if (clientes[i].socket == 0) {
			clientes[i].socket = cliente.socket;
			clientes[i].atentido = false;
			clientes[i].addr = cliente.addr;
			clientes[i].addrlen = cliente.addrlen;
			printf("Añadido a la lista de sockets como %d\n", i);
			return i;
		}
	}
	printf("Se superó el numero maximo de clientes");
	close(cliente.socket);
	return -1;
}

void quitarCliente(int i) {
	// Liberamos del array al cliente y cerramos su socket
	getpeername(clientes[i].socket, (struct sockaddr*) &clientes[i].addr,
			(socklen_t*) &clientes[i].addrlen);
	printf("Invitado desconectado , ip %s , puerto %d \n",
			inet_ntoa(clientes[i].addr.sin_addr),
			ntohs(clientes[i].addr.sin_port));
	close(clientes[i].socket);
	clientes[i].socket = 0;
}

void procesarNuevasConexiones() {
	// Aceptamos nueva conexion
	t_cliente cliente;
	cliente.addrlen = sizeof(cliente.addr);
	int socketNuevoCliente;
	socketNuevoCliente = accept(socketNuevasConexiones,
			(struct sockaddr *) &cliente.addr, (socklen_t*) &cliente.addrlen);
	printf("Nueva conexión , socket %d , ip is : %s , puerto : %d \n",
			socketNuevoCliente, inet_ntoa(cliente.addr.sin_addr),
			ntohs(cliente.addr.sin_port));
	agregarCliente(cliente);
}

int tieneLectura(int socket) {
	return FD_ISSET(socket, &socketsParaLectura);
}

int incorporarSockets() {
	// Reseteamos y añadimos al set los sockets disponibles
	FD_ZERO(&socketsParaLectura);
	FD_SET(socketNuevasConexiones, &socketsParaLectura);
	mayorDescriptor = socketNuevasConexiones;
	return incorporarClientes();
}

int incorporarClientes() {
	// Reseteamos y añadimos al set los sockets disponibles, NO atendidos
	int i, filedes;
	for (i = 0; i < MAXCLIENTS; i++) {
		filedes = clientes[i].socket;
		if (filedes > 0) {
			if (clientes[i].atentido == false) {
				FD_SET(clientes[i].socket, &socketsParaLectura);
				if (filedes > mayorDescriptor)
					mayorDescriptor = filedes;
			}
		}
	}
	return mayorDescriptor;
}
