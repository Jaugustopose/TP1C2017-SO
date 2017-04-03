/*
 * kernel.c
 *
 *  Created on: 2/4/2017
 *      Author: utnso
 */
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#define PUERTO 9090
#define BACKLOG 10

int main(void) {

	int servidor = crear_socket();
	int enlace = enlazar_a_puerto(servidor,8080);

	printf("Estoy escuchando\n");
	listen(servidor, 100);

	struct sockaddr_in direcciónCliente;
	int len =sizeof (struct sockaddr_in);
	int cliente = accept(servidor, (void*) &direcciónCliente, &len);

		printf("Recibí una conexión en %d!!\n", cliente);
		send(cliente, "Hola NetCat!", 13, 0);
		send(cliente, ":)\n", 4, 0);

		for(;;);
		return 0;
}

//Funciones creadas

int crear_socket(){
	int soc = socket(AF_INET,SOCK_STREAM,0);
	if (soc == -1){
		printf("El socket no se creo");
	}
	return soc;
}

int enlazar_a_puerto(int socket, int puerto){
	struct sockaddr_in miSocket;
	miSocket.sin_family = AF_INET;
	miSocket.sin_port = (in_port_t)htons(puerto);
	miSocket.sin_addr.s_addr = htonl(INADDR_ANY);

	int enlace = bind(socket, (struct sockaddr*) &miSocket, sizeof(miSocket));
	if (enlace == -1) {
		perror ("No se puede enlazar a este puerto, ya esta enlazado a otro buachin \n");
	}
	return enlace;

}

