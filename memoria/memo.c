#include "memo.h"

void cargarConfigFile(){
	char* pat = string_new();
	char cwd[1024]; // Variable donde voy a guardar el path absoluto hasta el /Debug
	string_append(&pat,getcwd(cwd,sizeof(cwd)));
	string_append(&pat,"/memo.cfg");
	t_config* configMemo = config_create(pat);
	if (config_has_property(configMemo, "PUERTO_KERNEL")){
		config.puerto_kernel = config_get_int_value(configMemo, "PUERTO_KERNEL");
		printf("config.PUERTO_KERNEL: %d\n", config.puerto_kernel);
	}
	if (config_has_property(configMemo, "IP_KERNEL")){
		config.ip_kernel = config_get_string_value(configMemo, "IP_KERNEL");
		printf("config.IP_KERNEL: %s\n", config.ip_kernel);
	}
	if (config_has_property(configMemo, "MARCOS")){
		config.marcos = config_get_int_value(configMemo, "MARCOS");
		printf("config.MARCOS: %d\n", config.marcos);
	}
	if (config_has_property(configMemo, "MARCO_SIZE")){
		config.marco_size = config_get_int_value(configMemo, "MARCO_SIZE");
		printf("config.MARCO_SIZE: %d\n", config.marco_size);
	}
}

void enviar_mensajes(int cliente, unsigned int length) {
	while (1) {
		char mensaje[length];
		fgets(mensaje, sizeof mensaje, stdin);
		send(cliente, mensaje, strlen(mensaje), 0);
	}
}

int conectar_con_server(int cliente, const struct sockaddr_in* direccionServidor) {
	return connect(cliente, (void*) &*direccionServidor, sizeof(*direccionServidor));
}

void recibir_mensajes_en_socket(int socket) {
	char* buf = malloc(1000);
	while (1) {
		int bytesRecibidos = recv(socket, buf, 1000, 0);
		if (bytesRecibidos < 0) {
			perror("Ha ocurrido un error al recibir un mensaje");
			exit(EXIT_FAILURE);
		} else if (bytesRecibidos == 0) {
			printf("Se terminó la conexión en el socket \n", socket);
			close(socket);
			exit(EXIT_FAILURE);
		} else {
			//Recibo mensaje e informo
			buf[bytesRecibidos] = '\0';
			printf("Recibí el mensaje de %i bytes: ", bytesRecibidos);
			puts(buf);
		}
	}
	free(buf);
}

//void inicializarMemoria() {
//	//Creo memoria y la relleno
//	tamanioMemoria = config.marcos * config.marco_size;
//	memoria = malloc(tamanioMemoria);
//	memset(memoria, '\0', tamanioMemoria);
//}

//TODO En el checkpoint 2 tendremos sólo un gran bloque y una gran página.
void inicializarTablaDeFrames(){

}

int buscarMarco(int pid, int nroPagina){
	//TODO implementar búsqueda de marco
	return 0;
}

int almacenarBytes(int pid, int nroPagina, int offset, int tamanio, char* buffer){
//memcpy(contenido,memoria+paginaBuscada->marcoUtilizado * config.tamanio_marco+pedido.offset,pedido.cantBytes);
	int marcoPagina = buscarMarco(pid, nroPagina);
	char* destino = memoria + tamanioTablaPagina + marcoPagina * config.marco_size + offset;
	memcpy(destino, buffer, tamanio);

	return EXIT_SUCCESS;
}

int main(void) {
	//Setea config_t config
	cargarConfigFile();

	//Inicializamos tabla invertida de páginas
	tablaPagina_t tablaPaginasInvertida[config.marcos];
	int i;
	for(i=0; i<config.marcos; ++i){
		tablaPaginasInvertida[i].pid = -10;
		tablaPaginasInvertida[i].nroPagina = -1;
	}

	tamanioTablaPagina = config.marcos * sizeof(tablaPagina_t);
	tamanioMemoria = config.marco_size * config.marcos;
	char* memoria = malloc(tamanioMemoria);
	memcpy(memoria, tablaPaginasInvertida, tamanioTablaPagina);

	free(memoria);
	return EXIT_SUCCESS;
}


//Main Checkpoint 1
//int main(void) {
//	int cliente;
//	//Servidor al cual conectarse
//	struct sockaddr_in direccionServidor;
//
//	//Setea config_t config
//	cargarConfigFile();
//
//	direccionServidor.sin_family = AF_INET;
//	direccionServidor.sin_addr.s_addr = inet_addr(config.ipKernel);
//	direccionServidor.sin_port = htons(config.puertoKernel);
//	memset(&(direccionServidor.sin_zero), '\0', 8);
//
//	cliente = socket(AF_INET, SOCK_STREAM, 0);
//
//	if (conectar_con_server(cliente, &direccionServidor) != 0) {
//		perror("Error al conectar con el Kernel");
//		return EXIT_FAILURE;
//	} else {
//		if (direccionServidor.sin_family == AF_INET) {
//			char* ipConectada = inet_ntoa(direccionServidor.sin_addr);
//			int puertoConectado = ntohs(direccionServidor.sin_port);
//			printf("Conectado con %s:%i\n", ipConectada, puertoConectado);
//		} else {
//			printf("La dirección no es IPv4");
//		}
//
//	}
//
//	recibir_mensajes_en_socket(cliente);
//
//	return EXIT_SUCCESS;
//}
