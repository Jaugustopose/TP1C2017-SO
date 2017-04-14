#include "cpu.h"



#define MAXBYTESREAD 100

//Provisorio: le puse dos para luego factorizar con la de Kernel :D
int crearSocketDos() {
	return socket(AF_INET, SOCK_STREAM, 0);
}

int conectarSocket(int socket, struct sockaddr_in* dirServidor)
{
	return connect(socket, (struct sockaddr*) &*dirServidor, sizeof(struct sockaddr));
}

void cerrarSocket(int socket)
{
   close(socket);
}

void cargarConfiguracion()
{
	char* pat = string_new();
	char cwd[1024]; // Variable donde voy a guardar el path absoluto hasta el /Debug
	string_append(&pat,getcwd(cwd,sizeof(cwd)));
	string_append(&pat,"/cpu.cfg");
	t_config* configCpu = config_create(pat);
	free(pat);
	if (config_has_property(configCpu, "IP_MEMORIA"))
		config.IP_MEMORIA = config_get_string_value(configCpu, "IP_MEMORIA");
	if (config_has_property(configCpu, "PUERTO_MEMORIA"))
		config.PUERTO_MEMORIA = config_get_int_value(configCpu, "PUERTO_MEMORIA");
	if (config_has_property(configCpu, "PUERTO_KERNEL"))
		config.PUERTO_KERNEL = config_get_int_value(configCpu, "PUERTO_KERNEL");
	if (config_has_property(configCpu, "IP_KERNEL"))
		config.IP_KERNEL = config_get_string_value(configCpu, "IP_KERNEL");
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

int main(void){

	struct sockaddr_in dirServidor;
	//char buf[MAXBYTESREAD];
	char* buf = malloc(5);
	int socket;

	cargarConfiguracion();

	if((socket = crearSocketDos()) == -1)
	{
		perror("No se creo el socket correctamente");
		exit(1);
	}

	//Configuro Servidor
	dirServidor.sin_family = AF_INET;
	dirServidor.sin_port = htons(config.PUERTO_KERNEL);
	dirServidor.sin_addr.s_addr =  INADDR_ANY;
	memset(&(dirServidor.sin_zero), '\0', 8);

	if(conectarSocket(socket, &dirServidor) == -1)
	{
		perror("No se pudo conectar");
		exit(1);
	}
	int bytesRecibidos =  recv(socket, buf, 4, 0);
	if(bytesRecibidos < 0)
		{
			perror("Fallo el recv");
			exit(1);
		}


	recibir_mensajes_en_socket(socket);

	return EXIT_SUCCESS;
}
