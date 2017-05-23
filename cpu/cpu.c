
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
	string_append(&pat,"/Debug/cpu.cfg");
	t_config* configCpu = config_create(pat);
	free(pat);
	if (config_has_property(configCpu, "IP_MEMORIA"))
	{
		config.IP_MEMORIA = config_get_string_value(configCpu, "IP_MEMORIA");
		printf("config.IP_MEMORIA: %s\n", config.IP_MEMORIA);
	}
	if (config_has_property(configCpu, "PUERTO_MEMORIA"))
	{
		config.PUERTO_MEMORIA = config_get_int_value(configCpu, "PUERTO_MEMORIA");
		printf("config.PUERTO_MEMORIA: %d\n", config.PUERTO_MEMORIA);
	}
	if (config_has_property(configCpu, "PUERTO_KERNEL"))
	{
		config.PUERTO_KERNEL = config_get_int_value(configCpu, "PUERTO_KERNEL");
		printf("config.PUERTO_KERNEL: %d\n", config.PUERTO_KERNEL);
	}
	if (config_has_property(configCpu, "IP_KERNEL"))
	{
		config.IP_KERNEL = config_get_string_value(configCpu, "IP_KERNEL");
		printf("config.IP_KERNEL: %s\n", config.IP_KERNEL);
	}
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

struct sockaddr_in crearDireccionParaCliente(unsigned short PORT, char* IP) {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(IP);
	direccionServidor.sin_port = htons(PORT);
	return direccionServidor;
}


void conectarConKernel() {

	//Handshake
	dirKernel = crearDireccionParaCliente(config.PUERTO_KERNEL, config.IP_KERNEL);

	kernel = socket_ws();
	connect_w(kernel, &dirKernel);
	printf("Conectado a Kernel");

	send(kernel,&identidad, sizeof(int),0);

}
void conectarConMemoria() {

	//Handshake
	dirMemoria = crearDireccionParaCliente(config.PUERTO_MEMORIA, config.IP_MEMORIA);
	memoria = socket_ws();
	connect_w(memoria, &dirMemoria);
	printf("Conectado a Memoria");

}

void solicitarTamanioPaginaAMemoria()
{
	//Hacer lo mismo que memoria haga con kernel
}

void deserializarPCB(t_PCB* pcbNuevo, t_PCB* pcbSerializado)
{

}

void obtenerPCB()
{
	pcbNuevo = malloc(sizeof(t_PCB));
	//char* pcbSerializado = deserializar(1); //le paso sock kernel

}

void obtenerSentencia(int* tamanio)
{
	//Una sentencia puede estar repartida en una o mas paginas
	int pid = pcbNuevo->PID;
	int inicioSentencia;
	int longitudSentencia;
	int nroPaginaInicial;

	//Esta informacion limita la sentencia
	t_sentencia* sentenciaLimites = list_get(pcbNuevo->indiceCodigo, pcbNuevo->contadorPrograma);
	inicioSentencia = sentenciaLimites->offset_inicio;
	longitudSentencia = sentenciaLimites->longitud;

    nroPaginaInicial = (int)(inicioSentencia / tamanioPaginas);

    int sentenciaEnUnaPagina = (inicioSentencia + longitudSentencia) > tamanioPaginas;

    if(sentenciaEnUnaPagina)
    {
		send(memoria,((int)AccionPedirSentencia),1,0);
		t_solicitud solicitud;
		solicitud.offset = inicioSentencia;
		solicitud.nroPagina = nroPaginaInicial;
		solicitud.size = longitudSentencia;

		char* solicitudRecibida = string_new();
		memcpy(solicitudRecibida, &solicitud, sizeof(t_solicitud));
		int tamanioSol = sizeof(t_solicitud);
		send(memoria, solicitudRecibida, tamanioSol, 0);
		free(solicitudRecibida);

		char* buffer = malloc(tamanioSol);
		char* serialSentencia = recv(memoria, buffer, tamanioSol, MSG_WAITALL);

		if(serialSentencia[tamanioSol - 1]=='\n'){

			serialSentencia[tamanioSol - 1]='\0';
		}

		char* sentenciaRecibida = malloc(tamanioSol+1);
		sentenciaRecibida[tamanioSol]='\0';
		memcpy(sentenciaRecibida,serialSentencia,tamanioSol);
		string_append(&sentencia, sentenciaRecibida);
		free(serialSentencia);
		free(sentenciaRecibida);


    }else{
    	//Entonces la sentencia esta en mas de una pagina


    	   // int byteUltimaPagCompleta = nroPaginaInicial * tamanioPaginas;
    	   // int primerosBytes = inicioSentencia - byteUltimaPagCompleta;
    	   // int tamanioSentenciaPag = tamanioPaginas - primerosBytes;

    }

}

void parsear(char* sentencia)
{

}
void pedirSentencia()
{
	int tamanio;

	sentencia = string_new();
	obtenerSentencia(&tamanio);
	parsear(sentencia);

	free(sentencia);

}

void recibirOrdenes(char* accion)
{

	switch((int)accion){

		case AccionObtenerPCB: //Recibir nuevo PCB
			obtenerPCB();
			break;

		case AccionPedirSentencia: //Obtener y parsear sentencias
			if(ejecutar){
				pedirSentencia();
			}
			break;
	}

}

void esperarProgramas()
{
	char* accion;
	ejecutar = 1;

		while (1) {
			recv(memoria, accion, 1, MSG_WAITALL);
			recibirOrdenes(accion);
			free(accion);
		}
}

int main(void){

	cargarConfiguracion();
	inicializarPrimitivas();
	conectarConKernel();
	esperarProgramas();
 //conectarConMemoria();
	solicitarTamanioPaginaAMemoria();

	return EXIT_SUCCESS;
}
