/*
 * kernel.c
 *
 *  Created on: 2/4/2017
 *      Author: utnso
 */
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {

}

//Funciones creadas

int crear_socket(){
	int soc = socket(PF_INET,SOCK_STREAM,0);
	if (soc == -1){
		printf("El socket no se creo");
	}
	return soc;
}

int enlazar_a_puerto(int socket, int puerto){
	struct sockaddr_in miSocket;
	miSocket.sin_family = PF_INET;
	miSocket.sin_port = (in_port_t)htons(puerto);
	miSocket.sin_addr.s_addr = htonl(INADDR_ANY);

	int enlace = bind(socket, (struct sockaddr*) &miSocket, sizeof(miSocket));
	if (enlace == -1) {
		perror ("No se puede enlazar a este puerto, ya esta enlazado a otro buachin \n");
	}
}

