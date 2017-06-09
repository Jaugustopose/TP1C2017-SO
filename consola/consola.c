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

	programa = fopen("/home/utnso/Escritorio/facil.ansisop","rb");
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

void cargar_y_enviar_archivo(int sock)
{
	char* bufferArchivo = 0;
	long length;
	FILE* archivo = fopen("/home/utnso/Escritorio/facil.ansisop","rb");

	if(archivo){
		fseek(archivo,0,SEEK_END);
		length = ftell(archivo);
		fseek(archivo,0,SEEK_SET);
		bufferArchivo = (char*)malloc((length +1)*sizeof(char));
		if(bufferArchivo)
		{
			fread(bufferArchivo,1,length,archivo);
			printf("He recibido %d bytes de contenido: %.*s\n",length, length + 1, bufferArchivo);

			//Creo el header antes de enviar mensaje

			t_header cabeza;
			cabeza.id = 1;
			cabeza.tamanio = length;

			//Serializo el buffer con el mensaje
			void* bufferSerializado = malloc(cabeza.tamanio + sizeof(t_header));
			memcpy(bufferSerializado, &cabeza.id, sizeof(int)); //PRIMERO EL ID
			memcpy(bufferSerializado + sizeof(cabeza.id), &cabeza.tamanio, sizeof(int)); //SEGUNDO EL TAMAÑO
			memcpy(bufferSerializado + sizeof(t_header), bufferArchivo, cabeza.tamanio); // TERCERA LA DATA

			send(sock,bufferSerializado,length,0); // Envio archivo serializado
		}else
		{
			fclose(archivo);
		}

	}


}

void crearPrograma()
{

    int cliente = crearSocket();

	pthread_t idHilo = pthread_self();
	printf("Soy hilo: %d\n", idHilo);


	struct sockaddr_in direccionServidor; //Creo y configuro el servidor
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(config.IP_KERNEL);
	direccionServidor.sin_port = htons(config.PUERTO_KERNEL);

	if(conectarSocket(cliente, &direccionServidor) != 0){ // no se está conectando al servidor
			perror("No se realizó la conexión");
	}

	send(cliente,&identidad, sizeof(int),0);
}

void limpiaMensajes()
{
	system("clear");
}
int main(void){

	system("clear");



    cargarConfiguracion();

    while(1){
	printf("Ingrese una acción a realizar\n");
		puts("1: Iniciar Programa");
		puts("2: Finalizar Programa");
		puts("3: Desconectar Consola");
		puts("4: Limpiar Mensajes");

		char accion[3];
		if (fgets(accion, sizeof(accion), stdin) == NULL) {
					printf("ERROR EN fgets !\n");
					return 1;
			}
		int codAccion = accion[0] - '0';


		switch (codAccion) {
					case iniciarPrograma:
						//crea un hilo (programa)
						printf("Iniciando!...\n");
						pthread_t unHilo;
						pthread_create(&unHilo, NULL, (void*) crearPrograma);
					break;

					case finalizarPrograma:
						//recibe un PID y mata ese hilo(programa) particular
						printf("Codificar finalizar!\n");
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
	return EXIT_SUCCESS;
}


