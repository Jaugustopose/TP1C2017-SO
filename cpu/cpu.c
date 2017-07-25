
#include "cpu.h"
#include "deserializador.h"

char* lecturaLargoMensajeASerializar(int sock){

	char* serialLargo = malloc(sizeof(int));
	recv(sock, serialLargo, sizeof(int), 0);
	int largo = char4ToInt(serialLargo);
	char* mensaje = malloc(largo);
	recv(sock, mensaje, largo, 0);
	free(serialLargo);

	return mensaje;
}

void enviarLargoMensajeASerializar(int sock, int largo, char* mensaje){

	char* serialLargo = intToChar4(largo);
	send(sock,serialLargo, sizeof(int), 0);
	send(sock, mensaje, largo, 0);

	free(serialLargo);
}

void recibirQuantumSleep(){

	int quantum;
	int bytes = recv(kernel, &quantum, sizeof(int), 0);
	quantumSleep = quantum;

}

bool puedo_terminar(){
	return terminar && !ejecutando;
}

bool hayOverflow(){
	return overflow ==-10;
}

void overflowException(int mensajeMemoria){

	//TODO: Desarrollar el manejo del overflow. Hay que finalizar proceso y demas.
	//Puede recibir: 0 (stackoverflow), 1(marcos insuficientes), otra cosa.

	if(lanzarOverflowExep){

			finalizar_programa(false);

			lanzarOverflowExep=false;
		}
}

void actualizarPC(t_PCB* pcb, t_puntero_instruccion pc) {

	pcb->contadorPrograma = (int)pc;
}

void finalizarProgramaVariableInvalida(){

	char* accionKernel = (char*)accionFinProceso;
	send(kernel, accionKernel, sizeof(accionKernel), 0);
	free(accionKernel);

	char* accionMemoria = (char*)accionFinProceso;
	send(memoria, accionMemoria, sizeof(accionMemoria), 0);
	free(accionMemoria);


	//Falta: Destruir PCB

	ejecutando = false;
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
		string_append(&pat, getcwd(cwd, sizeof(cwd)));
		if (string_contains(pat, "/Debug")) {
			string_append(&pat, "/cpu.cfg");
		} else {
			string_append(&pat, "/Debug/cpu.cfg");
		}
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
	recv(kernel, &algoritmo, sizeof(int32_t), 0);
	printf("Algoritmo: %d", algoritmo);

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
	if(pcbNuevo != NULL){
			destruir_PCB(pcbNuevo);
		}

	//Comunicacion con kernel
	char* bufferPCB = lecturaLargoMensajeASerializar(kernel);

	//Deserializacion del PCB
	pcbNuevo = malloc(sizeof(t_PCB));
	deserializar_PCB(pcbNuevo, bufferPCB);

	//Guardo variables como globales a CPU
	stack = pcbNuevo->stackPointer;
	cantidadPagCodigo = pcbNuevo->cantidadPaginas;

	free(bufferPCB);

}

void desalojarProceso()
{
	ejecutando = false;

	//Envio PCB al kernel
	serializar_PCB(pcbNuevo, kernel, accionDesalojarProceso);
}

void finalizar_programa(bool normalmente){

	if(normalmente){
			//Loguear mensaje

	}
	if((overflow == -10) || normalmente){

		//Avisar a Memoria que termino el proceso
		char* accionEnviarMemo = (char*)accionFinProceso;
	    send(memoria, accionEnviarMemo, sizeof(int), 0);
		free(accionEnviarMemo);

	}


	//Avisar a Kernel que termino el proceso
	char* accionEnviar = (char*)accionFinProceso;
    send(kernel, accionEnviar, sizeof(int), 0);
	free(accionEnviar);

	destruir_PCB(pcbNuevo);
	ejecutando = false;
	pcbNuevo = NULL;
}

void inicializarContexto()
{
	ejecutando = true;
	terminar = false;
	lanzarOverflowExep = false;
	pcbNuevo = NULL;

}
/**********FUNCIONES PARA MANEJO DE SENTENCIAS*********************************************************************/

void enviarSolicitudBytes(int pid, int pagina, int offset, int size) {
	if(!hayOverflow())
	{
	pedidoBytesMemoria_t pedido;
	pedido.pid = pid;
	pedido.nroPagina = pagina;
	pedido.offset = offset;
	pedido.tamanio = size;

	void* solicitud = malloc(sizeof(int) + sizeof(t_pedido));

	int codAccion = solicitarBytesAccion;
	memcpy(solicitud, &codAccion, sizeof(codAccion));
	memcpy(solicitud + sizeof(codAccion), &pedido, sizeof(pedidoBytesMemoria_t));
	int tamanio = sizeof(codAccion) + sizeof(pedidoBytesMemoria_t);
	//int tamanio = serializar_pedido_bytes(solicitud, pedido);

	send(memoria, solicitud, tamanio, 0);

	char* stackOverflow = malloc(sizeof(int));
	int bytesRecibidos = recv(memoria, stackOverflow, sizeof(int), 0);
	overflow = char4ToInt(stackOverflow);
    free(stackOverflow);

	if (hayOverflow()) {
		log_debug(debugLog, ANSI_COLOR_RED "OVERFLOW!");
		ejecutando= false;
		//overflowException(overflow);
	}
    free(solicitud);
	}
}

void enviarAlmacenarBytes(int pid, int pagina, int offset, int size, t_valor_variable valor) {
	if (!hayOverflow()) {
	pedidoBytesMemoria_t sub_pedido;
	sub_pedido.pid = pid;
	sub_pedido.nroPagina = pagina;
	sub_pedido.offset = offset;
	sub_pedido.tamanio = size;

	pedidoAlmacenarBytesMemoria_t pedido;
	pedido.pedidoBytes = sub_pedido;
	pedido.buffer = malloc(sizeof(sub_pedido.tamanio));
	pedido.buffer = valor;

	void* solicitud = malloc(sizeof(int) + sizeof(pedidoAlmacenarBytesMemoria_t));

	int codAccion = almacenarBytesAccion;
	memcpy(solicitud, &codAccion, sizeof(codAccion));
	memcpy(solicitud + sizeof(codAccion), &pedido, sizeof(pedidoAlmacenarBytesMemoria_t));
	int tamanio = sizeof(codAccion) + sizeof(pedidoAlmacenarBytesMemoria_t);

	send(memoria, solicitud, tamanio, 0);

	char* stackOverflow = malloc(sizeof(int));
	int bytesRecibidos = recv(memoria, stackOverflow, sizeof(int), 0);
	overflow = char4ToInt(stackOverflow);
    free(stackOverflow);

	if (hayOverflow()) {
		log_debug(debugLog, ANSI_COLOR_RED "OVERFLOW!");
		ejecutando = false;
		//overflowException(overflow);
	}
    free(solicitud);
	}
}

int longitudSentencia(t_sentencia* sentencia) {
	return (int)(sentencia->fin - sentencia->inicio);
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

	return sentenciaRel;
}



int esPaginaCompleta(int longitudRestante) {
	return longitudRestante >= tamanioPaginas;
}

void recibirPedazoDeSentencia(int size){

	char* sentenciaRecibida = malloc(size);
	recv(memoria, sentenciaRecibida, size, 0);
	sacarSaltoDeLinea(sentenciaRecibida, size);
	char* sentencia = malloc(size+1);
	sentencia[size]='\0';
	memcpy(sentencia,sentenciaRecibida,size);

	string_append(&sentenciaPedida, sentencia);

	free(sentenciaRecibida);
	free(sentencia);

}

void pedirPrimeraSentencia(t_sentencia* sentenciaRelativa, int pagina, int* longitudRestante) {

//if (!hayOverflow()) {
	 int tamanioPrimeraSentencia = minimo(*longitudRestante, tamanioPaginas - sentenciaRelativa->inicio);

//	 char* accion = (int)solicitarBytesAccion;
//	 send(memoria, accion, sizeof(accion), 0);
//	 free(accion);

	 enviarSolicitudBytes(pcbNuevo->PID, pagina, sentenciaRelativa->inicio,tamanioPrimeraSentencia);
	(*longitudRestante) -= tamanioPrimeraSentencia;

	recibirPedazoDeSentencia(tamanioPrimeraSentencia);
 //}
}

void pedirPaginaCompleta(int nroPagina) {

//	char* accion = (int)solicitarBytesAccion;
//	send(memoria, accion, sizeof(accion), 0);
//	free(accion);

	enviarSolicitudBytes(pcbNuevo->PID, nroPagina, 0, tamanioPaginas);
	recibirPedazoDeSentencia(tamanioPaginas);
}

void pedirUltimaSentencia(t_sentencia* sentenciaRelativa, int pagina, int longitudRestante) {

	enviarSolicitudBytes(pcbNuevo->PID, pagina, 0, longitudRestante);
	recibirPedazoDeSentencia(longitudRestante);

}

void obtenerSentencia(int* tamanio)
{
	/*Una sentencia puede estar repartida en una o mas paginas*/

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

	//free(sentenciaRelativa);

}

int sentenciaNoFinaliza(char* sentencia){
	return strcmp(sentencia,"end")!=0
		&& strcmp(sentencia,"\tend")!=0
		&& strcmp(sentencia,"\t\tend")!=0;
}

void finalizar_proceso(bool terminaNormalmente)
{
	if(terminaNormalmente)
	{
		log_debug(debugLog, ANSI_COLOR_GREEN "El proceso ansisop ejecutó su última instrucción." ANSI_COLOR_RESET);

		//EXIT CODE: Cierre normal
		pcbNuevo->exitCode = 0;
		serializar_PCB(pcbNuevo, kernel, accionFinProceso);

		ejecutando = false;
		destruir_PCB(pcbNuevo);
		pcbNuevo = NULL;

	}else
	{
		log_info(debugLogger,"Finalizando proceso cpu...");

		close(kernel);
		close(memoria);

		log_info(debugLogger,"CPU finalizó correctamente.");
		destruirLogs();
		exit(EXIT_SUCCESS);
	}
}

/***********FUNCIONES DEL CIRCUITO DE PARSEO DE SENTENCIAS*****************************************/

void parsear(char* sentencia)
{
	pcbNuevo->contadorPrograma++;

	analizadorLinea(sentencia, &funciones, &funcionesKernel);

	if(sentenciaNoFinaliza(sentencia)){

			int codAccion = accionFinInstruccion;
			void* buffer = malloc(sizeof(int));
			memcpy(buffer, &codAccion, sizeof(codAccion)); //CODIGO DE ACCION
			send(kernel, buffer, sizeof(codAccion), 0);

		}
		else
		{
			bool terminaNormalmente = true;
			finalizar_proceso(terminaNormalmente);
		}
}

void pedirSentencia()
{
	//Recibe del nucleo el quantum
	recibirQuantumSleep();

	if(!puedo_terminar()){

			int tamanio;
			//Espera este tiempo antes de empezar con la proxima sentencia
			usleep(quantumSleep*1000);
			sentenciaPedida = string_new();
			obtenerSentencia(&tamanio);

			if(!hayOverflow()){
				parsear(sentenciaPedida);
				free(sentenciaPedida);
			}
		}

}

void recibirOrdenes(int32_t accionRecibida)
{

	switch((int)accionRecibida){

		case accionObtenerPCB: //Recibir nuevo PCB del Kernel
			overflow = false;
			lanzarOverflowExep = false;
			ejecutando = true;
			obtenerPCB();
			break;

		case accionContinuarProceso: //Obtener y parsear sentencias

			if(!puedo_terminar()){
				pedirSentencia();
			}
			break;
		case accionDesalojarProceso: //Envio PCB al Kernel

			desalojarProceso();

			break;
		case accionException: //Overflow - Le paso un cero que indica overflow

		   overflowException(0);

			break;
		case accionError: //Overflow - Le paso un cero que indica overflow

			break;

		default:
			log_error(errorLog, "Llego cualquier cosa.");
			log_error(errorLog, "Llego la accion numero |%d| y no hay una accion definida para eso.", accionRecibida);
		     exit(EXIT_FAILURE);
			break;
	}

}

void esperarProgramas()
{
	int32_t accionRecibida;
	ejecutando = true;

		while (!puedo_terminar()) {

			int bytes = recv(kernel, &accionRecibida, sizeof(int32_t), MSG_WAITALL);
			recibirOrdenes(accionRecibida);
		}
}

int obtenerTamanioPagina(int memoria) {
	int valorRecibido;
	int idMensaje = 6;
	send(memoria, &idMensaje, sizeof(int32_t), 0);
	recv(memoria, &valorRecibido, sizeof(int32_t), 0);

	return valorRecibido;
}

void loggearFinDePrimitiva(char* primitiva) {

	log_debug(debugLog, "La primitiva |%s| finalizó OK.", primitiva);
}

void finalizar_todo() {

	log_info(infoLog,"Finalizando proceso cpu...");

	close(kernel);
	close(memoria);

	log_info(infoLog,"CPU finalizó correctamente.");
	//destruirLogs();
	exit(EXIT_SUCCESS);
}

void handler(int sign) {
	if (sign == SIGUSR1) {
		printf("CHAAAAAU SIGUSR1!!!!\n");
		log_debug(debugLog, "Me Boletearon!!");
		if(!ejecutando){

			//TODO: deberia avisarle a memoria?
//			int codAccion = accionQuantumInterrumpido;
//			void* buffer = malloc(sizeof(int));
//			memcpy(buffer, &codAccion, sizeof(codAccion));
//			send(kernel, buffer, sizeof(codAccion), 0);

			serializar_PCB(pcbNuevo, kernel, accionQuantumInterrumpido);
			//finalizar_proceso(false);
			finalizar_todo();

		}else{
			terminar = true;
			log_info(infoLog, "Esperando a que termine la ejecucion del programa actual...");

//			int codAccion = accionQuantumInterrumpido;
//			void* buffer = malloc(sizeof(int));
//			memcpy(buffer, &codAccion, sizeof(codAccion));
//			send(kernel, buffer, sizeof(codAccion), 0);

			serializar_PCB(pcbNuevo, kernel, accionQuantumInterrumpido);
		}
	}
}
//void finalizar_CPU()
//{
//
//}

//void finalizar_programa(int32_t cierre)
//{
//	switch(cierre)
//	{
//		case CasoOverflow:
//
//
//		break;
//
//		case CasoVariableInvalida:
//
//
//		break;
//
//		case CasoCierreNormal:
//
//
//		break;
//	}
//}

int main(void){

	signal(SIGUSR1, handler); //el progama sabe que cuando se recibe SIGUSR1,se ejecuta handler

	crearLog(string_from_format("cpu_%d", getpid()), "CPU", 1);
	log_debug(debugLog, "Iniciando proceso CPU, PID: %d.", getpid());

	identidadCpu = SOYCPU;

	cargarConfiguracion();
	inicializarPrimitivas();
	inicializarContexto();
	conectarConKernel();
    conectarConMemoria();
    tamanioPaginas = obtenerTamanioPagina(memoria);

    esperarProgramas();
    finalizar_todo();

	return EXIT_SUCCESS;
}
