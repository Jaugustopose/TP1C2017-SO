
#include "cpu.h"
#include "deserializador.h"

void lanzar_excepcion(t_PCB* pcb, int32_t codigoError)
{
	switch(codigoError){

		case ERROR_MEMORIA:
			log_info(infoLog,ANSI_COLOR_RED "Stack overflow! se intentó leer una dirección inválida." ANSI_COLOR_RESET);
			setear_exitCode(pcb, ERROR_MEMORIA);
		break;

		case ERROR_ASIGNAR_PAGINAS:
			log_info(infoLog,ANSI_COLOR_RED "No hay marcos suficientes para el proceso." ANSI_COLOR_RESET);
			setear_exitCode(pcb, ERROR_ASIGNAR_PAGINAS);
		break;

		case ERROR_SOLICITUD_HEAP:
			log_info(infoLog,ANSI_COLOR_RED "No hay marcos suficientes para el proceso." ANSI_COLOR_RESET);
			setear_exitCode(pcb, ERROR_SOLICITUD_HEAP);
		break;

		case ERROR_ESCRITURA:
			log_info(infoLog,ANSI_COLOR_RED "No hay permisos  para el archivo que se desea escribir." ANSI_COLOR_RESET);
			setear_exitCode(pcb, ERROR_ESCRITURA);
			break;

		case ERROR_PERMISOS:
			log_info(infoLog,ANSI_COLOR_RED "No hay permisos  para el archivo que se desea leer" ANSI_COLOR_RESET);
			setear_exitCode(pcb, ERROR_PERMISOS);
			break;

		case ERROR_ACCESO_ARCHIVO:
			log_info(infoLog,ANSI_COLOR_RED "El programa intento acceder a un archivo inexistente." ANSI_COLOR_RESET);
			setear_exitCode(pcb, ERROR_ACCESO_ARCHIVO);
			break;

		default: printf("LLEGO CUARLQUIER COSA\n");
	}
}

char* lecturaLargoMensajeASerializar(int32_t sock){

	char* serialLargo = malloc(sizeof(int32_t));
	recv(sock, serialLargo, sizeof(int32_t), 0);
	int32_t largo = char4ToInt(serialLargo);
	char* mensaje = malloc(largo);
	recv(sock, mensaje, largo, 0);
	free(serialLargo);

	return mensaje;
}

void enviarLargoMensajeASerializar(int32_t sock, int32_t largo, char* mensaje){

	char* serialLargo = intToChar4(largo);
	send(sock,serialLargo, sizeof(int32_t), 0);
	send(sock, mensaje, largo, 0);

	free(serialLargo);
}

void recibirQuantumSleep(){

	int32_t quantum;
	int32_t bytes = recv(kernel, &quantum, sizeof(int32_t), 0);
	quantumSleep = quantum;

}

bool puedo_terminar(){
	return terminar && !ejecutando;
}

bool hayOverflow(){
	return overflow ==-10;
}

void actualizarPC(t_PCB* pcb, t_puntero_instruccion pc) {

	pcb->contadorPrograma = (int32_t)pc;
}

int32_t minimo(int32_t a, int32_t b) {
	return a < b ? a : b;
}

void sacarSaltoDeLinea(char* texto, int32_t ultPos){
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

int32_t socket_ws() {
	int32_t sock;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		puts("Error al crear socket");
		exit(1);
	}
	return sock;
}

void connect_w(int32_t cliente, struct sockaddr_in* direccionServidor) {
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

	send(kernel,&identidadCpu, sizeof(int32_t),0);
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

int32_t obtener_tamanio_pagina(int32_t memoria) {
	int32_t valorRecibido;
	int32_t idMensaje = 6;
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
	serializar_PCB(pcbNuevo, kernel, accionFinProceso);
}

void inicializarContexto()
{
	ejecutando = true;
	terminar = false;
	error = false;
	pcbNuevo = NULL;

}
/**********FUNCIONES PARA MANEJO DE SENTENCIAS*********************************************************************/

void enviarSolicitudBytes(int32_t pid, int32_t pagina, int32_t offset, int32_t size) {
	if(!hayOverflow())
	{
	pedidoBytesMemoria_t pedido;
	pedido.pid = pid;
	pedido.nroPagina = pagina;
	pedido.offset = offset;
	pedido.tamanio = size;

	void* solicitud = malloc(sizeof(int32_t) + sizeof(t_pedido));

	int32_t codAccion = solicitarBytesAccion;
	memcpy(solicitud, &codAccion, sizeof(codAccion));
	memcpy(solicitud + sizeof(codAccion), &pedido, sizeof(pedidoBytesMemoria_t));
	int32_t tamanio = sizeof(codAccion) + sizeof(pedidoBytesMemoria_t);


	send(memoria, solicitud, tamanio, 0);

	char* stackOverflow = malloc(sizeof(int32_t));
	int32_t bytesRecibidos = recv(memoria, stackOverflow, sizeof(int32_t), 0);
	overflow = char4ToInt(stackOverflow);
    free(stackOverflow);

			if (hayOverflow()) {

				log_debug(debugLog, ANSI_COLOR_RED "OVERFLOW!");
				ejecutando= false;
				lanzar_excepcion(pcbNuevo, ERROR_MEMORIA);
				error = true;
			}

    	free(solicitud);
	}
}

void enviarAlmacenarBytes(int32_t pid, int32_t pagina, int32_t offset, int32_t size, t_valor_variable valor) {
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

	void* solicitud = malloc(sizeof(int32_t) + sizeof(pedidoAlmacenarBytesMemoria_t));

	int32_t codAccion = almacenarBytesAccion;
	memcpy(solicitud, &codAccion, sizeof(codAccion));
	memcpy(solicitud + sizeof(codAccion), &pedido, sizeof(pedidoAlmacenarBytesMemoria_t));
	int32_t tamanio = sizeof(codAccion) + sizeof(pedidoAlmacenarBytesMemoria_t);

	send(memoria, solicitud, tamanio, 0);

	char* stackOverflow = malloc(sizeof(int32_t));
	recv(memoria, stackOverflow, sizeof(int32_t), 0);
	overflow = char4ToInt(stackOverflow);
    free(stackOverflow);

		if (hayOverflow()) {
			log_debug(debugLog, ANSI_COLOR_RED "OVERFLOW!");
			ejecutando = false;
			error = true;
			lanzar_excepcion(pcbNuevo, overflow);

		}

     free(solicitud);
	}
}

int32_t longitudSentencia(t_sentencia* sentencia) {
	return (int32_t)(sentencia->fin - sentencia->inicio);
}

t_sentencia* obtenerSentenciaRelativa(int32_t* paginaInicioSentencia) {

	t_sentencia* sentenciaAbsoluta = list_get(pcbNuevo->indiceCodigo, pcbNuevo->contadorPrograma);
	t_sentencia* sentenciaRel = malloc(sizeof(t_sentencia));

	    int32_t inicioAbsoluto = sentenciaAbsoluta->inicio;
		int32_t paginaInicio = (int32_t) (inicioAbsoluto / tamanioPaginas);
		int32_t inicioRelativo = inicioAbsoluto % tamanioPaginas;
		sentenciaRel->inicio = inicioRelativo;
		sentenciaRel->fin = inicioRelativo + longitudSentencia(sentenciaAbsoluta);

		(*paginaInicioSentencia) = paginaInicio;

	return sentenciaRel;
}

int32_t esPaginaCompleta(int32_t longitudRestante) {
	return longitudRestante >= tamanioPaginas;
}

void recibirPedazoDeSentencia(int32_t size){

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

void pedirPrimeraSentencia(t_sentencia* sentenciaRelativa, int32_t pagina, int32_t* longitudRestante) {

//if (!hayOverflow()) {
	 int32_t tamanioPrimeraSentencia = minimo(*longitudRestante, tamanioPaginas - sentenciaRelativa->inicio);

//	 char* accion = (int32_t)solicitarBytesAccion;
//	 send(memoria, accion, sizeof(accion), 0);
//	 free(accion);

	 enviarSolicitudBytes(pcbNuevo->PID, pagina, sentenciaRelativa->inicio,tamanioPrimeraSentencia);
	(*longitudRestante) -= tamanioPrimeraSentencia;

	recibirPedazoDeSentencia(tamanioPrimeraSentencia);
 //}
}

void pedirPaginaCompleta(int32_t nroPagina) {

//	char* accion = (int32_t)solicitarBytesAccion;
//	send(memoria, accion, sizeof(accion), 0);
//	free(accion);

	enviarSolicitudBytes(pcbNuevo->PID, nroPagina, 0, tamanioPaginas);
	recibirPedazoDeSentencia(tamanioPaginas);
}

void pedirUltimaSentencia(t_sentencia* sentenciaRelativa, int32_t pagina, int32_t longitudRestante) {

	enviarSolicitudBytes(pcbNuevo->PID, pagina, 0, longitudRestante);
	recibirPedazoDeSentencia(longitudRestante);

}

void obtenerSentencia(int32_t* tamanio)
{
	/*Una sentencia puede estar repartida en una o mas paginas*/

	int32_t paginaAPedir;

	t_sentencia* sentenciaRelativa = obtenerSentenciaRelativa(&paginaAPedir);
	int32_t longitudRestante = longitudSentencia(sentenciaRelativa);
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

bool _esEspacio(char caracter){
	return caracter==' ' || caracter=='\t' || caracter=='\f' || caracter=='\r' || caracter=='\v';
}

char* _string_trim(char* texto){
    int i;
    while (_esEspacio (*texto)) texto++;   //Anda a el primer no-espacio
    for (i = strlen (texto) - 1; i>0 && (_esEspacio (texto[i])); i--);   //y de atras para adelante
    texto[i + 1] = '\0';
    return texto;
}

int32_t sentenciaNoFinaliza(char* sentencia){
	char *linea, *lineaTemporal = linea = strdup( sentencia );
	linea = strdup( _string_trim(linea) );
	int32_t retorno = string_starts_with(linea, "end");
	free(lineaTemporal);
	free(linea);
	return !retorno;
}

void finalizar_proceso(bool terminaNormalmente)
{
	log_debug(debugLog, ANSI_COLOR_GREEN "El proceso ansisop ejecutó su última instrucción." ANSI_COLOR_RESET);

	pcbNuevo->exitCode = FINALIZO_CORRECTAMENTE;
	serializar_PCB(pcbNuevo, kernel, accionFinProceso);

	/* TODO: Finalizar por SIGUR
	 * else{

		log_debug(debugLog, ANSI_COLOR_GREEN "El proceso ansisop finaliza por SIGUSR1" ANSI_COLOR_RESET);
		serializar_PCB(pcbNuevo, kernel, accionQuantumInterrumpido);
	}*/

	ejecutando = false;
	destruir_PCB(pcbNuevo);
	pcbNuevo = NULL;

}


/***********FUNCIONES DEL CIRCUITO DE PARSEO DE SENTENCIAS*****************************************/

void parsear(char* sentencia)
{
	pcbNuevo->contadorPrograma++;

	analizadorLinea(sentencia, &funciones, &funcionesKernel);

	if(sentenciaNoFinaliza(sentencia)){

		if(error){
			log_debug(debugLog, ANSI_COLOR_RED "El proceso ansisop finaliza por un error en el programa." ANSI_COLOR_RESET);
			serializar_PCB(pcbNuevo, kernel, accionError);
			error = false;
		}else{
			int32_t codAccion = accionFinInstruccion;
			void* buffer = malloc(sizeof(int32_t));
			memcpy(buffer, &codAccion, sizeof(codAccion)); //CODIGO DE ACCION
			send(kernel, buffer, sizeof(codAccion), 0);
		}

	}else{
		finalizar_proceso(true);
	}
}

void pedirSentencia()
{
	//Recibe del nucleo el quantum
	recibirQuantumSleep();

	if(!puedo_terminar()){

			int32_t tamanio;
			//Espera este tiempo antes de empezar con la proxima sentencia
			usleep(quantumSleep*1000);
			sentenciaPedida = string_new();
			obtenerSentencia(&tamanio);

			parsear(sentenciaPedida);
			free(sentenciaPedida);

		}

}

void recibirOrdenes(int32_t accionRecibida)
{

	switch((int32_t)accionRecibida){

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
			//ejecucionInterrumpida = false;
			break;
		case accionDesalojarProceso: //Envio PCB al Kernel

			desalojarProceso();

			break;
		case accionException: //Overflow - Le paso un cero que indica overflow

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

			int32_t bytes = recv(kernel, &accionRecibida, sizeof(int32_t), MSG_WAITALL);
			recibirOrdenes(accionRecibida);
		}
}

int32_t obtenerTamanioPagina(int32_t memoria) {
	int32_t valorRecibido;
	int32_t idMensaje = 6;
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

void handler(int32_t sign) {
	if (sign == SIGUSR1) {
		printf("CHAAAAAU SIGUSR1!!!!\n");
		log_debug(debugLog, "Me Boletearon!!");
		if(!ejecutando){

			//TODO: deberia avisarle a memoria?
//			int32_t codAccion = accionQuantumInterrumpido;
//			void* buffer = malloc(sizeof(int32_t));
//			memcpy(buffer, &codAccion, sizeof(codAccion));
//			send(kernel, buffer, sizeof(codAccion), 0);

			serializar_PCB(pcbNuevo, kernel, accionQuantumInterrumpido);
			finalizar_proceso(true);
			finalizar_todo();

		}else{
			terminar = true;
			log_info(infoLog, "Esperando a que termine la ejecucion del programa actual...");

//			int32_t codAccion = accionQuantumInterrumpido;
//			void* buffer = malloc(sizeof(int32_t));
//			memcpy(buffer, &codAccion, sizeof(codAccion));
//			send(kernel, buffer, sizeof(codAccion), 0);

			serializar_PCB(pcbNuevo, kernel, accionQuantumInterrumpido);
		}
	}
}

int32_t main(void){

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
