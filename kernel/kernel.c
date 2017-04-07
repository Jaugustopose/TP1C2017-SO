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

	int sockServ, sockClie; // Escuchar sobre sockServ, nuevas conexiones sobre sockAccept
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
	if ((sockClie = acceptearSocket(sockServ, sin_size, &clie_addr)) == -1) {
	printf("Error al tratar de acceptear");
	continue;
	}

	char mensaje[] = "Hola cliente! Como te va?\n";
	char* buffer = malloc(1000);

	printf("Server: Se establecio la coneccion con cliente %s\n", inet_ntoa(clie_addr.sin_addr));
	if (!fork()) { // Este es el proceso hijo que atiende al cliente

		close(sockServ); // El hijo no necesita este descriptor
		if (send(sockClie, mensaje, strlen(mensaje), 0) == -1) {
			printf("Error al sendear mensaje al cliente");
		}

		while (1){

			int bytesRecibidos = recv(sockClie, buffer, 1000, 0);
					if (bytesRecibidos <= 0) {
						perror("Se desconecto el cliente o siamo fuori de la copa");
						return 1;
					}

					buffer[bytesRecibidos] = '\0';
					printf("Me llegaron %d bytes con %s\n", bytesRecibidos, buffer);

		}


		/*uint32_t tamanioPaquete;
		recv(sockClie, &tamanioPaquete, 4, 0);

		char* buff = malloc(tamanioPaquete);
		recv(sockClie, buff, tamanioPaquete, MSG_WAITALL);
*/

	free(buffer);
	close(sockClie); // Como el padre no lo necesita, lo cierro
	}

	return 0;
	}
}
