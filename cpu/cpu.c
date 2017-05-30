
#include "cpu.h"


#define MAXBYTESREAD 100

t_identidad* identidadCpu = SOYCPU;

int minimo(int a, int b) {
	return a < b ? a : b;
}

void sacarSaltoDeLinea(char* texto, int ultPos){
	if(texto[ultPos-1]=='\n'){
		texto[ultPos-1]='\0';
	}
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

	send(kernel,&identidadCpu, sizeof(int),0);

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
	deserializar_PCB(&pcbNuevo, kernel);

}

void finalizarProceso(bool normalmente){

	char* accion = (char*)AccionFinProceso;
    send(kernel, accion, 1, 0);
	free(accion);

	//TODO:destruir pcb
	ejecutar = false;
	pcbNuevo = NULL;
}

/**********FUNCIONES PARA MANEJO DE SENTENCIAS*********************************************************************/

void enviarSolicitudSentencia(int pagina, int offset, int size) {

	t_pedido pedido;
	pedido.offset = offset;
	pedido.nroPagina = pagina;
	pedido.size = size;

	char* solicitud = string_new();

	//VER LA SERIALIZACION
	int tamanio = serializar_pedido(solicitud, &pedido);
	send(memoria, solicitud, tamanio, 0);

	free(solicitud);
}

t_sentencia* obtenerSentenciaRelativa(int* paginaInicioSentencia) {

	t_sentencia* sentenciaAbsoluta = list_get(pcbNuevo->indiceCodigo, pcbNuevo->contadorPrograma);
	t_sentencia* sentenciaRelativa = malloc(sizeof(t_sentencia));

	    int inicioAbsoluto = sentenciaAbsoluta->inicio;
		int paginaInicio = (int) (inicioAbsoluto / tamanioPaginas);
		int inicioRelativo = inicioAbsoluto % tamanioPaginas;
		sentenciaRelativa->inicio = inicioRelativo;
		sentenciaRelativa->fin = inicioRelativo + longitudSentencia(sentenciaAbsoluta);

		(*paginaInicioSentencia) = paginaInicio;

	return sentenciaRelativa;
}

int longitudSentencia(t_sentencia* sentencia) {
	return sentencia->fin - sentencia->inicio;
}

int esPaginaCompleta(int longitudRestante) {
	return longitudRestante >= tamanioPaginas;
}

void recibirPedazoDeSentencia(int size){

	char* buffer = malloc(size);
	recv(memoria, buffer, size, MSG_WAITALL);
	sacarSaltoDeLinea(buffer, size);
	char* sentencia = malloc(size+1);
	sentencia[size]='\0';
	memcpy(sentencia,buffer,size);

	string_append(&sentenciaPedida, sentencia);

	free(buffer);
	free(sentencia);

}

void pedirPrimeraSentencia(t_sentencia* sentenciaRelativa, int pagina, int* longitudRestante) {

 int tamanioPrimeraSentencia = minimo(*longitudRestante,	tamanioPaginas - sentenciaRelativa->inicio);

char* accion = (char*)AccionPedirSentencia;
send(memoria, accion, sizeof(accion), 0);
free(accion);

enviarSolicitudSentencia(pagina, sentenciaRelativa->inicio,tamanioPrimeraSentencia);
(*longitudRestante) = (int)(longitudRestante - tamanioPrimeraSentencia);

recibirPedazoDeSentencia(tamanioPrimeraSentencia);
}

void pedirPaginaCompleta(int nroPagina) {

	char* accion = (char)AccionPedirSentencia;
	send(memoria, accion, sizeof(accion), 0);
	free(accion);

	enviarSolicitudSentencia(nroPagina, 0, tamanioPaginas);
	recibirPedazoDeSentencia(tamanioPaginas);
}

void pedirUltimaSentencia(t_sentencia* sentenciaRelativa, int pagina, int longitudRestante) {

	char* accion = (char*)AccionPedirSentencia;
	send(memoria, accion, sizeof(accion), 0);
	free(accion);
	enviarSolicitudSentencia(pagina, 0, longitudRestante);
	recibirPedazoDeSentencia(longitudRestante);

}

void obtenerSentencia(int* tamanio)
{
	/*Una sentencia puede estar repartida en una o mas paginas*/

	int pid = pcbNuevo->PID;
	int inicioSentencia; //primer byte de la sentencia
	int longitudTotalSentencia; //desplazamiento desde el primer byte de la sentencia
	int paginaAPedir;

	t_sentencia* sentenciaRelativa = obtenerSentenciaRelativa(&paginaAPedir);
	int longitudRestante = longitudSentencia(sentenciaRelativa);
	(*tamanio) = longitudRestante;

	// Pido la primera pagina
	pedirPrimeraSentencia(sentenciaRelativa, paginaAPedir, &longitudRestante);
	paginaAPedir++;

	//Pido las paginas que siguen de forma completa
	while (esPaginaCompleta(longitudRestante)) {
		pedirPaginaCompleta(paginaAPedir);
		longitudRestante = longitudRestante - tamanioPaginas;
		paginaAPedir++;
	}

	//Si queda alguna pagina INCOMPLETA la pido
	if(longitudRestante>0){
		pedirUltimaSentencia(sentenciaRelativa, paginaAPedir, longitudRestante);
		paginaAPedir++;
	}

	free(sentenciaRelativa);

}

int sentenciaNoFinaliza(char* sentencia){
	return strcmp(sentencia,"end")!=0 && strcmp(sentencia,"\tend")!=0 && strcmp(sentencia,"\t\tend")!=0;
}

/***********FUNCIONES DEL CIRCUITO DE PARSEO DE SENTENCIAS*****************************************/

void parsear(char* sentencia)
{
	pcbNuevo->contadorPrograma++;

	if(sentenciaNoFinaliza(sentencia)){

	//Le paso sentencia, set de primitivas de CPU y set de primitivas de kernel
	analizadorLinea(sentencia, &funciones, &funcionesKernel);

	char* accion = (char*)AccionFinInstruccion;
	send(kernel, accion, 1, 0);
	free(accion);

	}else{
		  finalizarProceso(true);
	}
}

void pedirSentencia()
{
	int tamanio;

	sentenciaPedida = string_new();
	obtenerSentencia(&tamanio);
	parsear(sentenciaPedida);

	free(sentenciaPedida);

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
	ejecutar = true;

		while (1) {
			recv(kernel, accion, 1, MSG_WAITALL);
			recibirOrdenes(accion);
			free(accion);
		}
}

int main(void){

	cargarConfiguracion();
	inicializarPrimitivas();
	//conectarConKernel();
   // conectarConMemoria();
	//solicitarTamanioPaginaAMemoria();

	//PRUEBA GASTON: LUEGO BORRAR A LA MIERDA
	t_PCB* pcbFalso = malloc(sizeof(t_PCB));
	pcbFalso->PID = 3;
	pcbFalso->cantidadPaginas = 4;
	pcbFalso->contadorPrograma = 1;
//	pcbFalso->indiceCodigo = list_create();

	serializar_PCB(pcbFalso,1,9);


    esperarProgramas();


	return EXIT_SUCCESS;
}
