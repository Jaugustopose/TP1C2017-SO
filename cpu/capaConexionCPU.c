#include "capaConexionCPU.h"



int charToInt(char *c){
	return ((int)(*c));
}


int getHandshake(int cliente) {
	char* handshake = recv_nowait_ws(cliente, 1);
	return charToInt(handshake);
}

void conectarConKernel() {
	dirKernel = direccionParaCliente(config.PUERTO_KERNEL, config.IP_KERNEL);
	kernel = socket_ws();
	connect_w(kernel, &dirKernel);
	printf("Conectado a Nucleo");

	//Handshake
	char* hand = string_from_format("%c%c", 'AcaVaElHeader', SOYCPU);
		send_w(kernel, hand, 2);

		if (getHandshake(kernel) != SOYNUCLEO) {
			perror("Se esperaba que CPU se conecte con kernel.");
		} else {
			printf("Exito al hacer handshake con kernel.");
		}
}
void conectarConMemoria() {
	dirMemoria = direccionParaCliente(config.PUERTO_MEMORIA, config.IP_MEMORIA);
	memoria = socket_ws();
	connect_w(memoria, &dirMemoria);
	printf("Conectado a Memoria");

	//Handshake
		char* hand = string_from_format("%c%c", 'AcaVaElHeader', SOYCPU);
			send_w(memoria, hand, 2);

			if (getHandshake(memoria) != SOYMEMO) {
				perror("Se esperaba que CPU se conecte con memoria.");
			} else {
				printf("Exito al hacer handshake con memoria.");
			}
}
