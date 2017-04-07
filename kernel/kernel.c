/*
 * kernel.c
 *
 *  Created on: 2/4/2017
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#define MIPUERTO 9090
#define BACKLOG 10
void sigchld_handler(int s)
{
	while (wait(NULL) > 0);
}

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

int acceptearSocket(int sockServ, int sin_size, struct sockaddr_in* clie_addr) {
	return accept(sockServ, (struct sockaddr*) &*clie_addr, &sin_size);
}

int main(void)
{

	//VARIABLES

	int sockServ, sockAccept; // Escuchar sobre sockServ, nuevas conexiones sobre sockAccept
	struct sockaddr_in mi_addr; // información sobre mi dirección
	struct sockaddr_in clie_addr; // información sobre la dirección del cliente
	int sin_size;
	struct sigaction sa;
	int yes=1;

	//Creo el socket
	if ((sockServ = crearSocket()) == -1) {
	printf("Error al crear socket");
	exit(1);
	}

	//Lo hago reutilizable
	if (reusarSocket(sockServ, yes) == -1) {
	printf("Error al tratar de reusar socket");
	exit(1);
	}

	mi_addr.sin_family = AF_INET; // Ordenación de bytes de la máquina
	mi_addr.sin_port = htons(MIPUERTO); // short, Ordenación de bytes de la red
	mi_addr.sin_addr.s_addr = INADDR_ANY; // Rellenar con mi dirección IP
	memset(&(mi_addr.sin_zero), '\0', 8); // Poner ceros para rellenar el resto de la estructura


	//Bindear socket a cliente
	if (bindearSocket(sockServ, &mi_addr)== -1) {
	printf("Error al tratar de bindear");
	exit(1);
	}

	//Dejar socket escuchando
	if (listenearSocket(sockServ) == -1) {
	printf("Error al tratar de listenear");
	exit(1);
	}

	// Eliminar procesos muertos
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
	printf("Error al tratar de matar procesos zombies");
	exit(1);
	}

	//Loop para accept con todas las conexiones
	while(1) {
	sin_size = sizeof(struct sockaddr_in);
	if ((sockAccept = acceptearSocket(sockServ, sin_size, &clie_addr)) == -1) {
	printf("Error al tratar de acceptear");
	continue;
	}

	char mensaje[] = "Hola cliente! Como te va?\n";
	char* buffer = malloc(10);

	printf("Server: Se establecio la coneccion con cliente %s\n", inet_ntoa(clie_addr.sin_addr));
	if (!fork()) { // Este es el proceso hijo que atiende al cliente

		close(sockServ); // El hijo no necesita este descriptor
		if (send(sockAccept, mensaje, strlen(mensaje), 0) == -1) {
			printf("Error al sendear mensaje al cliente");
		}

		while(1) {
		int recibido = recv(sockAccept,buffer, 4,MSG_WAITALL);
		if (recibido <= 0) {
			printf("Error al recibir info del cliente");
			close(sockAccept);
			exit(0);
			}


		buffer[recibido] = '\0';
		printf("Me llegaron %d bytes con %s", recibido, buffer);
		}

	free(buffer);
	close(sockAccept); // Como el padre no lo necesita, lo cierro
	}

	return 0;
	}
}
