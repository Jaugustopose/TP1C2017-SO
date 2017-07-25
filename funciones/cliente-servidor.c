
#include "cliente-servidor.h"


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
		 printf("Error al tratar de bindear\n");
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
	struct sockaddr_in direccionServ;
	direccionServ.sin_family = AF_INET;
	direccionServ.sin_port = htons(PORT); // short, Ordenación de bytes de la red
	direccionServ.sin_addr.s_addr = INADDR_ANY;
	memset(&(direccionServ.sin_zero), '\0', 8); // Poner ceros para rellenar el resto de la estructura
	return direccionServ;
}

int conectar_con_server(int cliente, const struct sockaddr_in* direccionServidor) {
	printf("Intentando conectar al servidor\n");
	return connect(cliente, (struct sockaddr*) &*direccionServidor, sizeof(struct sockaddr));
}

/////////////////////////OPERACIONES VARIAS//////////////////////////////////////////

int redondear(float numero) {
		int resultado;
		if((numero - (int)numero) !=0){
			numero++;
			resultado = (int) numero;
		}else {
			resultado = (int)numero;
		}

		printf("%d\n",resultado); /* prints !!!Hello World!!! */
		return resultado;
}

