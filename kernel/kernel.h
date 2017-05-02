/*
 * kernel.h
 *
 *  Created on: 9/4/2017
 *      Author: utnso
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include <commons/config.h>
#include <commons/string.h>
#include <string.h>

struct configuracion {


	int PUERTO_CONSOLA;
	int PUERTO_FS;
	int PUERTO_MEMORIA;
	int PUERTO_CPU;
	int PUERTO_KERNEL;
	char* IP_MEMORIA;
	char* IP_FS;
	// FALTAN AGREGAR VARIABLES SEGUN AVANCE EL TP (SEMAFOROS, QUANTUM, ETC)
};
t_config* configKernel;
struct configuracion config;

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
	}else printf("Estoy escuchando\n");
}

struct sockaddr_in crearDireccionServidor(unsigned short PORT) {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_port = htons(config.PUERTO_KERNEL); // short, Ordenación de bytes de la red
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	memset(&(direccionServidor.sin_zero), '\0', 8); // Poner ceros para rellenar el resto de la estructura
	return direccionServidor;
}


char* procesarIdentidad(int sock){
	char* buf= malloc(sizeof(char));
	recv(sock,buf,sizeof(char),0);
	printf("hola estoy en el procesar ID\n");
	printf("El valor de la identidad del cliente es %c\n",&buf);
	return buf;
}

void colocarSegunBolsa(int sockClie, char identidad, fd_set bolsa1, fd_set bolsa2){

	switch(identidad){

		case 1: FD_SET(sockClie,&bolsa1); //agrego una nueva consola a la bolsa de consolas
				break;
		case 2: FD_SET(sockClie,&bolsa2); //agrego un nuevo cpu a la bolsa de cpus
				break;
	}
}


#endif
