
#include "cpu.h"


#define MAXBYTESREAD 100

t_identidad* identidadCpu = SOYCPU;

bool hayOverflow(){
	return overflow!=1;
}

void overflowException(int mensajeMemoria){

	//TODO: Desarrollar el manejo del overflow. Hay que finalizar proceso y demas.
}

void goToMagia(){};

void actualizarPC(t_PCB* pcb, t_puntero_instruccion pc) {

	pcb->contadorPrograma = (int)pc;
}

void finalizarProcesoVariableInvalida(){

	char* accionKernel = (char*)accionFinProceso;
	send(kernel, accionKernel, sizeof(accionKernel), 0);
	free(accionKernel);

	char* accionMemoria = (char*)accionFinProceso;
	send(memoria, accionMemoria, sizeof(accionMemoria), 0);
	free(accionMemoria);


	//Falta: Destruir PCB

	ejecutar = false;
	salteaCircuitoConGoTo = true;
	pcbNuevo = NULL;

}

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

int obtener_tamanio_pagina(int memoria) {
	int valorRecibido;
	int idMensaje = 6;
	send(memoria, &idMensaje, sizeof(int32_t), 0);
	recv(memoria, &valorRecibido, sizeof(int32_t), 0);

	return valorRecibido;
}

void deserializarPCB(t_PCB* pcbNuevo, t_PCB* pcbSerializado)
{

}

void obtenerPCB()
{
	pcbNuevo = malloc(sizeof(t_PCB));
	//deserializar_PCB(&pcbNuevo, kernel);
	stack = pcbNuevo->stackPointer;
	cantidadPagCodigo = pcbNuevo->cantidadPaginas;

}

void desalojarProceso()
{
	ejecutar = false;

	//Envio PCB al kernel
	int bytes = bytes_PCB(&pcbNuevo);

	serializar_PCB(&pcbNuevo, kernel, accionFinProceso);
}

void finalizarProceso(bool normalmente){

	char* accion = (char*)accionFinProceso;
    send(kernel, accion, 1, 0);
	free(accion);

	//TODO:destruir pcb
	ejecutar = false;
	pcbNuevo = NULL;
}

/**********FUNCIONES PARA MANEJO DE SENTENCIAS*********************************************************************/

void enviarSolicitudSentencia(int pid, int pagina, int offset, int size) {

	pedidoBytesMemoria_t pedido;
	pedido.pid = pid;
	pedido.nroPagina = pagina;
	pedido.offset = offset;
	pedido.tamanio = size;

	char* solicitud = string_new();

	//VER LA SERIALIZACION
	int tamanio = serializar_pedido_bytes(solicitud, pedido);
	send(memoria, solicitud, tamanio , 0);


}

t_sentencia* obtenerSentenciaRelativa(int* paginaInicioSentencia) {

	t_sentencia* sentenciaAbsoluta = list_get(pcbNuevo->indiceCodigo, pcbNuevo->contadorPrograma);
	t_sentencia* sentenciaRel = malloc(sizeof(t_sentencia));

	    int inicioAbsoluto = sentenciaAbsoluta->inicio;
		int paginaInicio = (int) (inicioAbsoluto / tamanioPaginas);
		int inicioRelativo = inicioAbsoluto % tamanioPaginas;
		sentenciaRel->inicio = inicioRelativo;
		sentenciaRel->fin = inicioRelativo + longitudSentencia(sentenciaAbsoluta);

		(*paginaInicioSentencia) = paginaInicio;

    free(sentenciaAbsoluta);
	return sentenciaRel;
}

int longitudSentencia(t_sentencia* sentencia) {
	return (int)(sentencia->fin - sentencia->inicio);
}

int esPaginaCompleta(int longitudRestante) {
	return longitudRestante >= tamanioPaginas;
}

void recibirPedazoDeSentencia(int size){

	char* alo = malloc(size);
	recv(memoria, alo, size, 0);
	sacarSaltoDeLinea(alo, size);
	char* sentencia = malloc(size+1);
	sentencia[size]='\0';
	memcpy(sentencia,alo,size);

	string_append(&sentenciaPedida, sentencia);

	free(alo);
	free(sentencia);

}

void pedirPrimeraSentencia(t_sentencia* sentenciaRelativa, int pagina, int* longitudRestante) {

 int tamanioPrimeraSentencia = minimo(*longitudRestante,	tamanioPaginas - sentenciaRelativa->inicio);

//char* accion = (char*)solicitarBytesAccion;
//send(memoria, accion, sizeof(accion), 0);
//free(accion);

enviarSolicitudSentencia(pcbNuevo->PID, pagina, sentenciaRelativa->inicio,tamanioPrimeraSentencia);
(*longitudRestante) = (int)(longitudRestante - tamanioPrimeraSentencia);

recibirPedazoDeSentencia(tamanioPrimeraSentencia);
}

void pedirPaginaCompleta(int nroPagina) {

	char* accion = (char)solicitarBytesAccion;
	send(memoria, accion, sizeof(accion), 0);
	free(accion);

	enviarSolicitudSentencia(pcbNuevo->PID, nroPagina, 0, tamanioPaginas);
	recibirPedazoDeSentencia(tamanioPaginas);
}

void pedirUltimaSentencia(t_sentencia* sentenciaRelativa, int pagina, int longitudRestante) {

	char* accion = (char*)solicitarBytesAccion;
	send(memoria, accion, sizeof(accion), 0);
	free(accion);
	enviarSolicitudSentencia(pcbNuevo->PID, pagina, 0, longitudRestante);
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

	char* accion = (char*)accionFinInstruccion;
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

		case accionObtenerPCB: //Recibir nuevo PCB del Kernel
			obtenerPCB();
			break;

		case accionContinuarProceso: //Obtener y parsear sentencias

			if(ejecutar){
				pedirSentencia();
			}

			break;
		case accionFinProceso: //Envio PCB al Kernel

			desalojarProceso();

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

//	crearLog(string_from_format("cpu_%d", getpid()),"CPU",0);
//	log_info(infoLog, "Iniciando proceso CPU, PID: %d.", getpid());
//
//	logger = log_create("cpu.log", "CPU", false, LOG_LEVEL_INFO);
//	debugLogger = log_create("cpu.log", "CPU", false, LOG_LEVEL_DEBUG);
//
//	log_info(logger, "Inicio CPU");
//	log_debug(debugLogger, "Inicio CPU");

	cargarConfiguracion();
	inicializarPrimitivas();
	//conectarConKernel();
    // conectarConMemoria();
     //tamanioPaginas = obtener_tamanio_pagina(memoria);
     tamanioPaginas = 32;


	//PRUEBA GASTON: LUEGO BORRAR A LA MIERDA
	t_PCB* pcbFalso = malloc(sizeof(t_PCB));
	pcbFalso->PID = 2;
	pcbFalso->cantidadPaginas = 1;
	pcbFalso->contadorPrograma = 0;
	pcbFalso->indiceCodigo = list_create();
	pcbFalso->indiceEtiquetas = dictionary_create();
//	t_sentencia* sentencia = malloc(sizeof(t_sentencia));
//	sentencia->inicio = 0;
//	sentencia->fin = 14;
//	list_add(pcbFalso->indiceCodigo,sentencia);

	pcbFalso->stackPointer = stack_crear();
	t_elemento_stack* elemento = stack_elemento_crear();
	stack_push(pcbFalso->stackPointer, elemento);

	char* PROGRAMA =
			 "#!/usr/bin/ansisop\n"
			"begin\n"
			"variables a, b\n"
			"a = 3\n"
			"b = 5\n"
			"a = b + 12\n"
			"end\n";

	char* PROGRAMA2 = "#!/usr/bin/ansisop\n"
			"begin\n"
				":etiqueta\n"
				"wait c\n"
				"print !colas\n"
				"signal b\n"
				"#Ciclar indefinidamente\n"
				"goto etiqueta\n"
		     	"end\n";

	t_metadata_program* metadata = metadata_desde_literal(PROGRAMA);
	t_sentencia* sentencia;

	int i;
	for (i = 0; i < metadata->instrucciones_size; i++) {

			sentencia = malloc(sizeof(t_sentencia));
			sentencia->inicio = metadata->instrucciones_serializado[i].start;
			sentencia->fin = sentencia->inicio + metadata->instrucciones_serializado[i].offset;
			list_add(pcbFalso->indiceCodigo, sentencia);
		}

	int longitud = 0;

		for (i = 0; i < metadata->etiquetas_size; i++) {

			if (metadata->etiquetas[i] == '\0') {

				char* etiqueta = malloc(longitud + 1);
				memcpy(etiqueta, metadata->etiquetas + i - longitud, longitud + 1);

				int* salto = malloc(sizeof(int));
				memcpy(salto, metadata->etiquetas + i + 1, sizeof(int));

				dictionary_put(pcbFalso->indiceEtiquetas, etiqueta, salto);

				i = i + sizeof(int);
				longitud = 0;
			} else
				longitud++;
		}
		free(metadata);


	pcbNuevo = pcbFalso;
	t_PCB* pcbUltimo = malloc(sizeof(t_PCB));

	char* pcbSerial;
	pcbSerial = serializar_PCB(pcbNuevo,kernel, accionFinProceso);
	deserializar_PCB(pcbUltimo, kernel, pcbSerial);
	t_PCB* pcbResultado = pcbUltimo;


//	sentenciaPedida = string_new();
//	enviarSolicitudSentencia(2,2,2,5);
//	recibirPedazoDeSentencia(tamanioPaginas);

 //	pedirSentencia();

  // esperarProgramas();
   // destruirLogs();

	return EXIT_SUCCESS;
}
