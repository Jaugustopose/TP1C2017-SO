#include "consola.h"
#include "cliente-servidor.h"
#include "serializador.h"


void cargarConfiguracion(){


	char* pat = string_new();
		char cwd[1024]; // Variable donde voy a guardar el path absoluto hasta el /Debug
		string_append(&pat,getcwd(cwd,sizeof(cwd)));
		string_append(&pat,"/Debug/consola.cfg");
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

int main (void){

	int identidad = 1;


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
		return EXIT_FAILURE;
	}

	send(cliente,&identidad, sizeof(int),0);

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

				//Serializar y enviar archivo
				void* bufferSerializado = serializar(cabeza, bufferArchivo);
				send(cliente,bufferSerializado, sizeof(t_header) + (cabeza.tamanio),0);


			}else
			{
				fclose(archivo);
			}
		}



	while (1) {

			recibir_mensajes_en_socket(cliente);


		}



	return 0;
}


