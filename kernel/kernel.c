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
#define MAXDATASIZE 100 // Máximo número de bytes que se pueden leer de una vez.
#define MIPUERTO 9090
#define BACKLOG 10

int main(void) {

	//VARIABLES

	int servidor;
	int sin_size;


	servidor = crear_socket();

	int activado = 1; //Para poder reusar el mismo socket y no esperar 2 minutos.
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	int enlace = bindear(servidor,MIPUERTO);
	if (enlace == -1){
		printf("Error al bindear, dale que podes!");
	}

	int escucha = listen(servidor, BACKLOG);
	if (escucha == -1){
		printf("No se pudo relizar el listen, metele que se hace!");
	}
	printf("Estoy escuchando\n");

	struct sockaddr_in direccionCliente;
	sin_size = sizeof(struct sockaddr_in);
	int cliente = accept(servidor, (struct sockaddr*) &direccionCliente, &sin_size);

		printf("Recibí una conexión en %d!!\n", cliente);

		char* mensaje = "Holan te tengo fe!\n";
		int len = strlen(mensaje);

		int bytes_enviados = send(cliente, mensaje, len, 0);
		if (bytes_enviados == -1){
			printf("El send tuvo un error, revisalo que seguro lo arreglas!");
		}

		char buff[MAXDATASIZE];
		int recibido = recv(cliente,buff,MAXDATASIZE-1,0);
		if (recibido == -1){
			printf("Error con syscall recv, pero como dijo michetti: levantate siempre");
		}
		printf("Me llegaron %d bytes que decian %s", recibido,buff );
		free(buff);

		return 0;
}

//Funciones creadas

int crear_socket(){
	int soc = socket(AF_INET,SOCK_STREAM,0);
	if (soc == -1){
		printf("El socket no se creo, pero metele que estas ahi!");
	}
	return soc;
}

int bindear(int socket, int puerto){
	struct sockaddr_in miSocket;
	miSocket.sin_family = AF_INET;
	miSocket.sin_port = htons(MIPUERTO);
	miSocket.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(&(miSocket.sin_zero), '\0', 8); //Poner cero al resto de la estructura

	int enlace = bind(socket, (struct sockaddr*) &miSocket, sizeof(miSocket));
	if (enlace == -1) {
		perror ("No se puede enlazar a este puerto, ya esta enlazado a otro buachin \n");
	}
	return enlace;

}

