#include "consola.h"
#include "cliente-servidor.h"
#include "serializador.h"


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
 }

void cargarConfiguracion(){


	char* pat = string_new();
		char cwd[1024]; // Variable donde voy a guardar el path absoluto hasta el /Debug
		string_append(&pat,getcwd(cwd,sizeof(cwd)));
		string_append(&pat,"/consola.cfg");
		printf("El directorio sobre el que se esta trabajando es %s\n", pat);
		t_config* configConsola = config_create(pat);
		free(pat);
		if (config_has_property(configConsola, "IP_KERNEL")){
				config.IP_KERNEL = config_get_string_value(configConsola,"IP_KERNEL");
		printf("config.IP_KERNEL: %s\n", config.IP_KERNEL);
		}

		if (config_has_property(configConsola, "PUERTO_KERNEL")){
				config.PUERTO_KERNEL = config_get_int_value(configConsola,"PUERTO_KERNEL");
		printf("config.PUERTO_KERNEL: %d\n", config.PUERTO_KERNEL);
		}
}

int conectarSocket(int socket, struct sockaddr_in* direccionServidor){

	return connect(socket, (struct sockaddr*) &*direccionServidor, sizeof(struct sockaddr));

}

char* convertirArchivoACodigo()
{
	char* contenido = string_new();

	size_t largo = 0;
	char* linea = NULL;
	ssize_t lectura = getline(&linea, &largo, programa);

	   //Mientras haya algo que leer, entonces lee
		while (lectura != -1) {

			string_append(&contenido, linea);

			//free(linea);

			largo = 0; //no se si es necesario... pero nunca sobra xD
			lectura = getline(&linea, &largo, programa);
		}

		//free(linea);

		string_append(&contenido, "\0");


		return contenido;

}

struct sockaddr_in crearDireccionParaCliente(unsigned short PORT, char* IP) {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(IP);
	direccionServidor.sin_port = htons(PORT);
	return direccionServidor;
}

int socket_ws() {
	int sock;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		puts("Error al crear socket");
		exit(1);
	}
	return sock;
}

void connect_w(int cliente, struct sockaddr_in* direccionServidor) {
	if (connect(cliente, (void*) direccionServidor, sizeof(*direccionServidor))
			!= 0) {
		perror("No se pudo conectar");
		exit(1);
	}
}

void conectarConKernel() {

	//Handshake
	dirKernel = crearDireccionParaCliente(config.PUERTO_KERNEL, config.IP_KERNEL);

	kernel = socket_ws();
	connect_w(kernel, &dirKernel);
	send(kernel,&identidad, sizeof(int),0);

}

void crearPrograma()
{

    int cliente = crearSocket();

	pthread_t idHilo = pthread_self();
	printf("Soy hilo: %d\n", idHilo);

//	struct sockaddr_in direccionServidor; //Creo y configuro el servidor
//	direccionServidor.sin_family = AF_INET;
//	direccionServidor.sin_addr.s_addr = inet_addr(config.IP_KERNEL);
//	direccionServidor.sin_port = htons(config.PUERTO_KERNEL);
//
//	if(conectarSocket(cliente, &direccionServidor) != 0){ // no se está conectando al servidor
//			perror("No se realizó la conexión");
//	}
//
//	send(cliente,&identidad, sizeof(int),0);

	conectarConKernel();

	int pidRecibido;
	recv(cliente, &pidRecibido, sizeof(int32_t), MSG_WAITALL);

	list_add(&listaPIDs, pidRecibido);

	printf("PID: %d\n", pidRecibido);
}

void limpiaMensajes()
{
	system("clear");
}

void imprimeMenuUsuario()
{
	   printf("Ingrese una acción a realizar\n");
			puts("1: Iniciar Programa");
			puts("2: Finalizar Programa");
			puts("3: Desconectar Consola");
			puts("4: Limpiar Mensajes");
}

void pidePathAlUsuario()
{
  printf("Ingresar archivo ansisop: ");
  scanf("%c", path);

  programa = fopen("/home/utnso/Escritorio/facil.ansisop","rb");
  if(programa == NULL){
	  perror("No se encontro el archivo.");
    }
  else
    {
	  convertirArchivoACodigo();
    }

}

void crearHiloPrograma()
{
	pthread_t unHilo;
	pthread_create(&unHilo, NULL, (void*)crearPrograma);
}

void atenderAcciones(char* accionRecibida){

	switch ((int)accionRecibida) {

		case accionError:
			break;

		case accionImprimirTextoConsola:

			break;

		case accionConsolaFinalizarNormalmente:

			break;

		case accionConsolaFinalizarErrorInstruccion:
			exit(EXIT_SUCCESS);
			break;

		default:

			exit(EXIT_FAILURE);
			break;

	}
}

/****Esta funcion es para probar nada mas**/
void imprimirPIDs(int pid){

	char* new = string_from_format("PID:%d ", pid);
	string_append(&programasExec, new);
	free(new);
}

void imprimirProgramasEnEjecucion(){
	programasExec = string_new();
	list_iterate(&listaPIDs,(void*)imprimirPIDs);
	printf("PIDs Impresos: %s", programasExec);
	free(programasExec);
}

void escucharUsuario()
{
	 while(1){

	    	imprimeMenuUsuario();

			char accion[3];
			if (fgets(accion, sizeof(accion), stdin) == NULL) {
						printf("ERROR EN fgets !\n");
				}
			int codAccion = accion[0] - '0';


			switch (codAccion) {
						case iniciarPrograma:
							//crea un hilo (programa)
							pidePathAlUsuario();
							printf("Iniciando!...\n");
							crearHiloPrograma();
							limpiaMensajes();
							imprimeMenuUsuario();
						break;

						case finalizarPrograma:
							//recibe un PID y mata ese hilo(programa) particular
							imprimirProgramasEnEjecucion();
						break;

						case desconectarConsola:
							//Matar todos los threads del kernel abortivamente
							printf("Codificar desconectar!\n");
						break;

						case limpiarMensajes:
							limpiaMensajes();
						break;

				}
	 }
}

void escucharPedidosKernel()
{
	while (1) {
			char* accionRecibida = malloc(sizeof(int));
			recv(kernel, accionRecibida, sizeof(char), 0);
			atenderAcciones(accionRecibida);
			free(accionRecibida);
		}
}

int main(void){

	limpiaMensajes();

    cargarConfiguracion();

    escucharUsuario();

    escucharPedidosKernel();

	return EXIT_SUCCESS;
}


