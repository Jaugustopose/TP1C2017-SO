/*
 * cliente-servidor.h
 *
 *  Created on: 29/4/2017
 *      Author: utnso
 */

#ifndef CLIENTE_SERVIDOR_H_
#define CLIENTE_SERVIDOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int crearSocket() {
	int sock;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1){
		puts("Error al crear socket");
			exit(1);
	}
	else return sock;
}

void reusarSocket(int sockServ, int yes) {
	if ((setsockopt(sockServ, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)))== -1) {
		printf("Error al tratar de reusar socket");
			exit(1);
	};
}

void bind_w(int sockServ, const struct sockaddr_in* mi_addr) {
	int bin;
	 bin = bind(sockServ, (struct sockaddr*) &*mi_addr, sizeof(struct sockaddr));
	 if (bin == -1) {
		 printf("Error al tratar de bindear");
		 	exit(1);
	 }
}

void listen_w(int sockServ) {
	int list;
	list = listen(sockServ, 10);
	if (list ==-1) {
		printf("Error al tratar de dejar el socket listeneando");
		exit(1);
	}
}

struct sockaddr_in crearDireccionServidor(unsigned short PORT) {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_port = htons(PORT); // short, Ordenación de bytes de la red
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	memset(&(direccionServidor.sin_zero), '\0', 8); // Poner ceros para rellenar el resto de la estructura
	return direccionServidor;
}



#endif /* CLIENTE_SERVIDOR_H_ */
