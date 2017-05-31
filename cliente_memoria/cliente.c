/*
 * cliente.c
 *
 *  Created on: 20/5/2017
 *      Author: utnso
 */

#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
//#include "deserializador.h"
#include "serializador.h"
#include "cliente.h"

void enviar_mensajes(int cliente, unsigned int length) {
//	while (1) {
//		printf("Ingrese un código de acción\n");
//		char mensaje[length];
//		fgets(mensaje, sizeof mensaje, stdin);
//		send(cliente, mensaje, strlen(mensaje), 0);
//	}
	char* endptr;
	while (1) {
		printf("Ingrese un código de acción\n");
		char mensaje[length];
		fgets(mensaje, sizeof mensaje, stdin);
		int* mensajeInt = strtol(mensaje, &endptr, 10);
		printf("MensajeInt: %d\n", mensajeInt);
		send(cliente, &mensajeInt, sizeof(mensajeInt), 0);
	}
}

int conectar_con_server(int cliente, const struct sockaddr_in* direccionServidor) {
	printf("Intentando conectar al servidor\n");
	return connect(cliente, (struct sockaddr*) &*direccionServidor, sizeof(struct sockaddr));
}

void recibir_mensajes_en_socket(int socket) {
	char* buf = malloc(1000);
	while (1) {
		int bytesRecibidos = recv(socket, buf, 1000, 0);
		if (bytesRecibidos < 0) {
			perror("Ha ocurrido un error al recibir un mensaje");
			exit(EXIT_FAILURE);
		} else if (bytesRecibidos == 0) {
			printf("Se terminó la conexión en el socket %d\n", socket);
			close(socket);
			exit(EXIT_FAILURE);
		} else {
			//Recibo mensaje e informo
			buf[bytesRecibidos] = '\0';
			printf("Recibí el mensaje de %i bytes: ", bytesRecibidos);
			puts(buf);
		}
	}
	free(buf);
}

void pedidoInicializarPrograma(int cliente) {
	int codigoAccion = 1;
	pedidoSolicitudPaginas_t pedidoPaginas;
	pedidoPaginas.pid = 2;
	pedidoPaginas.cantidadPaginas = 10;
	printf("Sizeof pedidoPaginas: %d\n", sizeof(pedidoPaginas));
	char* buffer = serializarMemoria(codigoAccion, &pedidoPaginas,
			sizeof(pedidoPaginas));
	printf("buffer serializado: %d\n", *buffer);
	send(cliente, buffer, sizeof(codigoAccion) + sizeof(pedidoPaginas), 0);
	free(buffer);
	//Reservo para recibir un int con el resultAccion
	int resultAccion;
	recv(cliente, &resultAccion, sizeof(int), 0);
	printf("inicializarPrograma resultó con código de acción: %d\n", resultAccion);
}

void enviarSolicitudAlmacenarBytes(int cliente) {
	char* buffer2;
	int codigoAccion = 3;
	pedidoAlmacenarBytesMemoria_t pedidoAlmacenar;
	pedidoAlmacenar.pedidoBytes.pid = 2;
	pedidoAlmacenar.pedidoBytes.nroPagina = 2;
	pedidoAlmacenar.pedidoBytes.offset = 0;
	pedidoAlmacenar.pedidoBytes.tamanio = 5;
	pedidoAlmacenar.buffer = "hola";
	printf("pedidoAlmacenar.buffer: %s\n", pedidoAlmacenar.buffer);

	//No paso sizeof(pedidoAlmacenar) porque el último campo es un puntero y me dará siempre 4 bytes. Entonces lo armo en dos partes,
	// y en la segunda el tamaño es el que indica pedidoAlmacenar.pedidoBytes.tamanio.
	// Luego, en el send, armo la suma que me da la cantidad de bytes correctos (para que no tome el puntero, sino el verdadero tamaño
	buffer2 = serializarMemoria(codigoAccion, &pedidoAlmacenar.pedidoBytes, sizeof(pedidoAlmacenar.pedidoBytes));
	memcpy(buffer2 + sizeof(codigoAccion) + sizeof(pedidoAlmacenar.pedidoBytes), pedidoAlmacenar.buffer, pedidoAlmacenar.pedidoBytes.tamanio);
	printf("luego de serializar: %s\n", buffer2 + sizeof(codigoAccion) + sizeof(pedidoAlmacenar.pedidoBytes));

	send(cliente, buffer2, sizeof(codigoAccion) + sizeof(pedidoAlmacenar.pedidoBytes) + pedidoAlmacenar.pedidoBytes.tamanio, 0);

	//Ahora recibo la respuesta
	int resultAccion;
	recv(cliente, &resultAccion, sizeof(resultAccion), 0);
	printf("almacenarBytes resultó con código de acción: %d\n", resultAccion);

//	free(buffer2);
}

void enviarPedidoBytes(int cliente) {

	char* buffer3;
	char* bytesRespuesta;
	int codigoAccion = 4;
	pedidoBytesMemoria_t pedidoBytes;
	pedidoBytes.pid = 2;
	pedidoBytes.nroPagina = 2;
	pedidoBytes.offset = 0;
	pedidoBytes.tamanio = 5;
	buffer3 = serializarMemoria(codigoAccion, &pedidoBytes, sizeof(pedidoBytes));

	send(cliente, buffer3, sizeof(codigoAccion) + sizeof(pedidoBytes), 0);

	//Ahora recibo la respuesta
	bytesRespuesta = malloc(pedidoBytes.tamanio);
	recv(cliente, bytesRespuesta, pedidoBytes.tamanio, 0);
	printf("solicitarBytes devolvió la siguiente cadena de bytes: %s\n", bytesRespuesta);
	free(bytesRespuesta);

}

void pedidoFinalizarPrograma(int cliente) {
	int codigoAccion = 5;
	int pidAFinalizar = 2;
	char* buffer = serializarMemoria(codigoAccion, &pidAFinalizar, sizeof(pidAFinalizar));
	printf("pedidoFinalizarPrograma - buffer serializado: %s\n", buffer);
	send(cliente, buffer, sizeof(codigoAccion) + sizeof(pidAFinalizar), 0);
	free(buffer);
	//Reservo para recibir un int con el resultAccion
	int resultAccion;
	recv(cliente, &resultAccion, sizeof(int), 0);
	printf("finalizarPrograma resultó con código de acción: %d\n", resultAccion);
}

int main(void) {
	int cliente;
	//Servidor al cual conectarse
	struct sockaddr_in direccionServidor;

	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr("127.0.0.1");
	direccionServidor.sin_port = htons(9030);

	cliente = socket(AF_INET, SOCK_STREAM, 0);
	if (cliente == -1) {
		perror("No se pudo crear el socket correctamente");
		return EXIT_FAILURE;
	}
	printf("Socket creado\n");
	if (conectar_con_server(cliente, &direccionServidor) != 0) {
		perror("Error al conectar con el Kernel");
		return EXIT_FAILURE;
	} else {
		printf("Conectado al server\n");
		if (direccionServidor.sin_family == AF_INET) {
			char* ipConectada = inet_ntoa(direccionServidor.sin_addr);
			int puertoConectado = ntohs(direccionServidor.sin_port);
			printf("Conectado con %s:%i\n", ipConectada, puertoConectado);
		} else {
			printf("La dirección no es IPv4");
		}

	}

	pedidoInicializarPrograma(cliente);

	sleep(1);

	enviarSolicitudAlmacenarBytes(cliente);

	sleep(1);

	enviarPedidoBytes(cliente);

//	sleep(1);
//
//	pedidoFinalizarPrograma(cliente);

	close(cliente);
	return EXIT_SUCCESS;
}
