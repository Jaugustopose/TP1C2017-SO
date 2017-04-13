
#include "consola.h"


void cargarConfiguracion(){

	struct configuracion config;
	t_config* configConsola = config_create("../consola.cfg");
	config.IP_KERNEL = config_get_string_value(configConsola, "IP_KERNEL");
	config.PUERTO_KERNEL = config_get_int_value(configConsola, "PUERTO_KERNEL");
     //return 1;
}

int crearSocket(){
	return socket(AF_INET,SOCK_STREAM,0);
}

int conectarSocket(int socket, struct sockaddr_in* direccionServidor){
	return connect(socket, (struct sockaddr*) &*direccionServidor, sizeof(struct sockaddr));
}

void cerrarSocket(int socket){
	close(socket);
}

int main (void){

	//if(cargarConfiguracion() == 1){  para mas adelante se eliminará el if, solo se puso para verificar que estaba
		//printf("esta Ok");
	//}
    cargarConfiguracion();

    int cliente = crearSocket();
	if(cliente == -1){  // Se valida de que se pudo crear el socket sin inconvenientes, retornando de "crearSocket un valor >=0.En caso de devolver -1 se visualizará el error.
		perror("No se pudo crear el socket correctamente");
	}

	struct sockaddr_in direccionServidor; //Creo y configuro el servidor
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(config.IP_KERNEL);
	direccionServidor.sin_port = htons(config.PUERTO_KERNEL);

	if(conectarSocket(cliente, &direccionServidor) != 0){ // no se está conectando al servidor
		perror("No se realizó la conexión");
	}

	while (1) {
	char mensaje[1000];
	scanf("%s", mensaje);
	send(cliente, mensaje, strlen(mensaje), 0);
	}


	return 0;
}
