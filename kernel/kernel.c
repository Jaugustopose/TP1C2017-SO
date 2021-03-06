#include "kernel.h"
#include "funcionesKernel.h"
#include "gestionDeProcesos.h"
#include "CapaFS.h"
#include "primitivasKernel.h"

void crearSemaforos();
void crearCompartidas();
void iniciarVigilanciaConfiguracion();

/********************************** INICIALIZACIONES *****************************************************/

void cargarConfiguracion(char* path) {
	/*char* path = string_new();
	char cwd[1024]; // Variable donde voy a guardar el path absoluto hasta el /Debug
	string_append(&path, getcwd(cwd, sizeof(cwd)));
	if (string_contains(path, "/Debug")){
		string_append(&path,"/kernel.cfg");
	}else{
	string_append(&path, "/Debug/kernel.cfg");
	}*/
	configKernel = config_create(path);
	//free(path);

	if (config_has_property(configKernel, "IP_MEMORIA")) {
		config.IP_MEMORIA = config_get_string_value(configKernel, "IP_MEMORIA");
		printf("IP_MEMORIA: %s\n", config.IP_MEMORIA);
	}
	if (config_has_property(configKernel, "IP_FS")) {
		config.IP_FS = config_get_string_value(configKernel, "IP_FS");
		printf("IP_FS: %s\n", config.IP_FS);
	}
	if (config_has_property(configKernel, "PUERTO_KERNEL")) {
		config.PUERTO_KERNEL = config_get_int_value(configKernel,
				"PUERTO_KERNEL");
		printf("PUERTO_KERNEL: %d\n", config.PUERTO_KERNEL);
	}
	if (config_has_property(configKernel, "PUERTO_MEMORIA")) {
		config.PUERTO_MEMORIA = config_get_int_value(configKernel,
				"PUERTO_MEMORIA");
		printf("PUERTO_MEMORIA: %d\n", config.PUERTO_MEMORIA);
	}
	if (config_has_property(configKernel, "PUERTO_CPU")) {
		config.PUERTO_CPU = config_get_int_value(configKernel, "PUERTO_CPU");
		printf("PUERTO_CPU: %d\n", config.PUERTO_CPU);
	}
	if (config_has_property(configKernel, "PUERTO_FS")) {
		config.PUERTO_FS = config_get_int_value(configKernel, "PUERTO_FS");
		printf("PUERTO_FS: %d\n", config.PUERTO_FS);
	}
	if (config_has_property(configKernel, "PUERTO_CONSOLA")) {
		config.PUERTO_CONSOLA = config_get_int_value(configKernel,
				"PUERTO_CONSOLA");
		printf("PUERTO_CONSOLA: %d\n", config.PUERTO_CONSOLA);
	}
	if (config_has_property(configKernel, "ALGORITMO")) {
			config.ALGORITMO = config_get_string_value(configKernel, "ALGORITMO");
			printf("ALGORITMO: %s\n", config.ALGORITMO);
		}
	if (config_has_property(configKernel, "QUANTUM")) {
			config.QUANTUM = config_get_int_value(configKernel,
					"QUANTUM");
			printf("QUANTUM: %d\n", config.QUANTUM);
		}
	if (config_has_property(configKernel, "QUANTUM_SLEEP")) {
				config.QUANTUM_SLEEP = config_get_int_value(configKernel,
						"QUANTUM_SLEEP");
				printf("QUANTUM_SLEEP: %d\n", config.QUANTUM_SLEEP);
			}
	if (config_has_property(configKernel, "GRADO_MULTIPROG")) {
		config.GRADO_MULTIPROG = config_get_int_value(configKernel,
				"GRADO_MULTIPROG");
		printf("GRADO_MULTIPROG: %d\n", config.GRADO_MULTIPROG);
	}
	if (config_has_property(configKernel, "SEM_IDS")) {
		config.SEM_IDS = config_get_array_value(configKernel, "SEM_IDS");
		printf("SEM_IDS: ");
		char **sem = config.SEM_IDS;
		int i;
		for (i = 0; sem[i]; ++i) {
			if(i!=0)
				printf("|");
			printf("%s",sem[i]);
		}
		printf("\n");
	}
	if (config_has_property(configKernel, "SEM_INIT")) {
		config.SEM_INIT = config_get_array_value(configKernel, "SEM_INIT");
		printf("SEM_INIT: ");
		char **sem = config.SEM_INIT;
		int i;
		for (i = 0; sem[i]; ++i) {
			if(i!=0)
				printf("|");
			printf("%s",sem[i]);
		}
		printf("\n");
	}
	if (config_has_property(configKernel, "SHARED_VARS")) {
		config.SHARED_VARS = config_get_array_value(configKernel,
				"SHARED_VARS");
		printf("SHARED_VARS: ");
		char **sem = config.SHARED_VARS;
		int i;
		for (i = 0; sem[i]; ++i) {
			if(i!=0)
				printf("|");
			printf("%s",sem[i]);
		}
		printf("\n");
	}
	if (config_has_property(configKernel, "STACK_SIZE")) {
			config.STACK_SIZE = config_get_int_value(configKernel,
					"STACK_SIZE");
			printf("STACK_SIZE: %d\n", config.STACK_SIZE);
		}
}




void enviar_stack_size(int sock)
{
	int codigoAccion = accionEnviarStackSize;
	int stackSize = config.STACK_SIZE;

	void* buffer = malloc(sizeof(int32_t)*2);
	memcpy(buffer, &codigoAccion, sizeof(codigoAccion));
	memcpy(buffer + sizeof(codigoAccion), &stackSize, sizeof(codigoAccion));

	send(sock, buffer, sizeof(int32_t)*2, MSG_WAITALL);

	//VERIFICAR
	free(buffer);
}

int obtener_tamanio_pagina(int memoria) {
	int valorRecibido;
	int idMensaje = 6;
	send(memoria, &idMensaje, sizeof(int32_t), MSG_WAITALL);
	recv(memoria, &valorRecibido, sizeof(int32_t), MSG_WAITALL);

	return valorRecibido;
}

void inicializarContexto() {
	listaDeProcesos = list_create();
	tablaSemaforos = dictionary_create();
	tablaCompartidas = dictionary_create();
	crearSemaforos();
	crearCompartidas();
	iniciarVigilanciaConfiguracion();
	listaPidHEAP = list_create();

	colaCPU = queue_create();
	colaNew = queue_create();
	colaReady = queue_create();
	colaExit = queue_create();
	planificacionDetenida = 0;

}

int pedido_Inicializar_Programa(int cliente, int paginas, int idProceso) {

	int codigoAccion = inicializarProgramaAccion;
	pedidoSolicitudPaginas_t pedidoPaginas;
	pedidoPaginas.pid = idProceso;
	pedidoPaginas.cantidadPaginas = paginas;
	printf("Sizeof pedidoPaginas: %d\n", sizeof(pedidoPaginas));
	void* buffer = serializarMemoria(codigoAccion, &pedidoPaginas, sizeof(pedidoPaginas));
	send(cliente, buffer, sizeof(codigoAccion) + sizeof(pedidoPaginas), 0);
	free(buffer);
	//Reservo para recibir un int con el resultAccion
	int resultAccion;
	recv(cliente, &resultAccion, sizeof(int), MSG_WAITALL);
	printf("inicializarPrograma resultó con código de acción: %d\n",resultAccion);

	return resultAccion;
}

int enviarSolicitudAlmacenarBytes(int memoria, t_proceso* unProceso, void* buffer, int tamanioTotal) {
	printf("tamanioTotal = %d\n", tamanioTotal);
	int codigoAccion = almacenarBytesAccion;
	//El STACK_SIZE se envia a memoria en handshake y memoria sabe que SIEMPRE debera reservar lo que le solicite
	//kernel en cantidadPaginasDeCodigo MAS el  stack_size que ya conoce.
	int tamanioBufferParaMemoria = ((sizeof(codigoAccion) + sizeof(pedidoBytesMemoria_t)) * unProceso->PCB->cantidadPaginas) + tamanioTotal;
	printf("tamanioBufferParaMemoria: %d\n", tamanioBufferParaMemoria);
	void* bufferParaAlmacenarEnMemoria = malloc(tamanioBufferParaMemoria);
	int m;
	int tamanioAAlmacenar;
	int start=0, posicionUltimoByteEscrito = 0;


	int resultAccion;

	for (m = 0; (m < (unProceso -> PCB->cantidadPaginas)) & (tamanioTotal != 0); m++) {

		if (tamanioTotal > tamanioPag) {

			tamanioAAlmacenar = tamanioPag;
			tamanioTotal = tamanioTotal - tamanioPag;
		} else{

			tamanioAAlmacenar = tamanioTotal;
			tamanioTotal = 0;
		}
		int tamanioBufferAux = (sizeof(codigoAccion)+sizeof(pedidoBytesMemoria_t)+tamanioAAlmacenar);
		pedidoAlmacenarBytesMemoria_t pedidoAlmacenar;
		pedidoAlmacenar.pedidoBytes.pid = unProceso->PCB->PID;
		pedidoAlmacenar.pedidoBytes.nroPagina = m;
		pedidoAlmacenar.pedidoBytes.offset = 0;
		pedidoAlmacenar.pedidoBytes.tamanio = tamanioAAlmacenar;
//		pedidoAlmacenar.buffer = string_substring_from(buffer,start);
		pedidoAlmacenar.buffer = (void*)string_substring(buffer,start,pedidoAlmacenar.pedidoBytes.tamanio);
		start = start + tamanioPag;


		if (m != 0) {
			printf("memcpy para m == %d\n", m);
			printf("corrimiento: %d\n", tamanioBufferAux * m);
			printf("tamanioBufferAux: %d\n", tamanioBufferAux);
			memcpy(bufferParaAlmacenarEnMemoria + posicionUltimoByteEscrito, &codigoAccion,sizeof(codigoAccion));
			memcpy(bufferParaAlmacenarEnMemoria + posicionUltimoByteEscrito + sizeof(int32_t), &pedidoAlmacenar.pedidoBytes.pid,sizeof(pedidoAlmacenar.pedidoBytes.pid));
			memcpy(bufferParaAlmacenarEnMemoria + posicionUltimoByteEscrito + sizeof(int32_t)*2, &pedidoAlmacenar.pedidoBytes.nroPagina,sizeof(pedidoAlmacenar.pedidoBytes.nroPagina));
			memcpy(bufferParaAlmacenarEnMemoria + posicionUltimoByteEscrito + sizeof(int32_t)*3, &pedidoAlmacenar.pedidoBytes.offset,sizeof(pedidoAlmacenar.pedidoBytes.offset));
			memcpy(bufferParaAlmacenarEnMemoria + posicionUltimoByteEscrito + sizeof(int32_t)*4, &pedidoAlmacenar.pedidoBytes.tamanio,sizeof(pedidoAlmacenar.pedidoBytes.tamanio));
			memcpy(bufferParaAlmacenarEnMemoria + posicionUltimoByteEscrito + sizeof(int32_t)*5, pedidoAlmacenar.buffer, pedidoAlmacenar.pedidoBytes.tamanio);
			posicionUltimoByteEscrito = posicionUltimoByteEscrito + tamanioBufferAux;

		} else {
			memcpy(bufferParaAlmacenarEnMemoria,&codigoAccion,sizeof(codigoAccion));
			memcpy(bufferParaAlmacenarEnMemoria + sizeof(int32_t), &pedidoAlmacenar.pedidoBytes.pid,sizeof(pedidoAlmacenar.pedidoBytes.pid));
			memcpy(bufferParaAlmacenarEnMemoria + sizeof(int32_t)*2, &pedidoAlmacenar.pedidoBytes.nroPagina,sizeof(pedidoAlmacenar.pedidoBytes.nroPagina));
			memcpy(bufferParaAlmacenarEnMemoria + sizeof(int32_t)*3, &pedidoAlmacenar.pedidoBytes.offset,sizeof(pedidoAlmacenar.pedidoBytes.offset));
			memcpy(bufferParaAlmacenarEnMemoria + sizeof(int32_t)*4, &pedidoAlmacenar.pedidoBytes.tamanio,sizeof(pedidoAlmacenar.pedidoBytes.tamanio));
			memcpy(bufferParaAlmacenarEnMemoria + sizeof(int32_t)*5, pedidoAlmacenar.buffer, pedidoAlmacenar.pedidoBytes.tamanio);
			posicionUltimoByteEscrito = tamanioBufferAux;
		}

	}

	int bytesEnviados = send(memoria, bufferParaAlmacenarEnMemoria, tamanioBufferParaMemoria,MSG_WAITALL);
	int k;
	for (k=0; k < m; k++){
		recv(memoria, &resultAccion, sizeof(resultAccion), MSG_WAITALL);
		printf("almacenarBytes resultó con código de acción: %d\n", resultAccion);
	}

	//VERIFICAR
	free(bufferParaAlmacenarEnMemoria);

	return resultAccion;

}

void comprobarSockets(int maxSock, fd_set* read_fds) {
	if (select(maxSock + 1, read_fds, NULL, NULL, NULL) == -1) { //Compruebo los sockets al mismo tiempo. Los NULL son para los writefds, exceptfds y el timeval.
		perror("select");
		exit(1);
	}
}

void procesos_exit_code_corto_consola(int fileDescriptor, t_list* listaConProcesos) {

	//Encontrar cada proceso con proceso.ConsolaDuenio = fileDescriptor
	//Cambiarle el exit code a -6 (finalizo por desconexion de consola)
	//Llevo el proceso con exit code = 6 a la cola de exit
	int fdClienteCont;
	for (fdClienteCont = 0; fdClienteCont < list_size(listaConProcesos); fdClienteCont++) {
		t_proceso* proceso = list_get(listaConProcesos, fdClienteCont);
		if (proceso -> ConsolaDuenio == fileDescriptor) {
			//queue_push(colaExit, proceso);
			proceso->abortado = true;
		}

	}
}

void liberar_procesos_de_cpu(int fileDescriptor, t_list* listaConProcesos) {

	//Encontrar cada proceso con proceso.CpuDuenio = filedescriptor
	//Y cambiarle el CpuDuenio a -1 (el menos 1 significa que no tiene cpu asignado)
	int fdClienteCont;
	for (fdClienteCont = 0; fdClienteCont < list_size(listaConProcesos); fdClienteCont++) {
		t_proceso* proceso = list_get(listaConProcesos, fdClienteCont);
		if (proceso -> CpuDuenio == fileDescriptor) {
			proceso -> CpuDuenio = -1;
		}

	}

}
void enviar_algoritmo_a_cpu()
{
	int algoritmo = (strcmp(config.ALGORITMO, FIFO) == 0)? SOY_FIFO : SOY_RR;
	send(sockClie, &algoritmo, sizeof(int32_t), MSG_WAITALL);
}
void Colocar_en_respectivo_fdset() {
	//Recibo identidad y coloco en la bolsa correspondiente
	recv(sockClie, &identidadCliente, sizeof(int32_t), MSG_WAITALL);
	switch (identidadCliente) {

	case soyConsola:

		FD_SET(sockClie, &bolsaConsolas); //agrego una nueva consola a la bolsa de consolas
		printf("Se ha conectado una nueva consola \n");
		break;

	case soyCPU:
			FD_SET(sockClie, &bolsaCpus); //agrego un nuevo cpu a la bolsa de cpus
			encolarCPU(colaCPU, sockClie);
			enviar_algoritmo_a_cpu();
			printf("Se ha conectado un nuevo CPU  \n");
			break;
	}

	if (sockClie > maxFd) {
		maxFd = sockClie;
	}
}

void conexion_de_cliente_finalizada() {
	// error o conexión cerrada por el cliente
	if (cantBytes == 0) {
		// conexión cerrada
		printf("Server: socket %d termino la conexion\n", fdCliente);
	} else {
		perror("recv");
	}
	// Eliminar del conjunto maestro y su respectiva bolsa
	FD_CLR(fdCliente, &master);
	if (FD_ISSET(fdCliente, &bolsaConsolas)) {
		FD_CLR(fdCliente, &bolsaConsolas);
		printf("Se desconecto consola del socket %d", fdCliente);

		procesos_exit_code_corto_consola(fdCliente, listaDeProcesos);

	} else {
		FD_CLR(fdCliente, &bolsaCpus);
		printf("Se desconecto cpu del socket %d\n", fdCliente);
	    colaCPU = queue_remove(colaCPU, fdCliente);
		liberar_procesos_de_cpu(fdCliente, listaDeProcesos);
	}
	close(fdCliente); // Si se perdio la conexion, la cierro.
}

void pedirMemoria(t_proceso* proceso)
{
	//CALCULO Y SOLICITO LAS PAGINAS QUE NECESITA EL SCRIPT//
	int paginasASolicitar = redondear((float) proceso->tamanioScript /(float) tamanioPag);
	int resultadoAccionInicializar = pedido_Inicializar_Programa(memoria,paginasASolicitar, proceso->pidProceso);
	if (resultadoAccionInicializar == 0) {//Depende de lo que devuelve si sale bien. (valor de EXIT_SUCCESS)

		proceso->PCB = crearPCB(proceso, paginasASolicitar);
		printf("Se procede a preparar solicitud almacenar para enviar\n");

		int resultadoAccionAlmacenar = enviarSolicitudAlmacenarBytes(memoria, proceso, proceso->codigoPrograma, proceso->tamanioScript);

			if (resultadoAccionAlmacenar == 0) {
				cambiarEstado(proceso, READY);
				proceso->PCB->cantidadPaginas = paginasASolicitar;
			 }
		}
	else{

		//Le creo el PCB solo para el exit code. Turbio, pero real.
		proceso->PCB = crearPCB(proceso, paginasASolicitar);
		proceso->PCB->exitCode = ERROR_RECURSOS;
		cambiarEstado(proceso, EXIT);
		list_add(listaDeProcesos, proceso);

	}

}


void Accion_envio_script(int memoria, int consola, int idMensaje)
{
	int tamanioScript;
	printf("Procediendo a recibir tamaño script\n");
	recv(consola, &tamanioScript, sizeof(int32_t), MSG_WAITALL);
	printf("Tamaño del script: %d\n", tamanioScript);
	char* buff = malloc(tamanioScript + 1);
	recv(fdCliente, buff, tamanioScript, MSG_WAITALL);
	printf("el valor de cadena es: %.*s\n", tamanioScript, buff);

    identificadorProceso++;
	t_proceso* proceso = crearProceso(identificadorProceso, consola, (char*)buff, tamanioScript);
    list_add(listaDeProcesos, proceso);
    queue_push(colaNew, proceso);
    send(consola, &identificadorProceso, sizeof(int32_t),MSG_WAITALL);

	if (config.GRADO_MULTIPROG > queue_size(colaReady) && !queue_is_empty(colaNew)) {
		t_proceso* proceso = (t_proceso*)queue_pop(colaNew);
		pedirMemoria(proceso);
	}

}

void sigusr1(int cpu){
	t_proceso* proceso = obtenerProceso(cpu);

	if (proceso!=NULL){
		proceso->PCB = recibirPCBDeCPU(cpu);
		proceso->sigusr1=true;
		planificarExpulsion(proceso);
	}
}

void atender_accion_cpu(int idMensaje, int memoria, int socketFS) {

	switch (idMensaje) {

	case accionSignal:
		recibirSignal(fdCliente);
		break;

	case accionFinInstruccion:
		rafagaProceso(fdCliente);
	break;

	case accionFinProceso:
		recibirFinalizacion(fdCliente);
	break;

	case accionError:
		recibirFinalizacionErronea(fdCliente);
	 break;

	case accionAsignarValorCompartida:
		obtenerAsignarCompartida(fdCliente);
	break;

	case accionObtenerValorCompartida:
		obtenerValorCompartida(fdCliente);
	break;

	case accionWait:
		recibirWait(fdCliente);
	break;

	case accionEscribir:
		escribirArchivo(fdCliente, socketFS);
		break;

	case accionMoverCursor:
		moverCursor(fdCliente, socketFS);
		break;

	case accionAbrirArchivo:
		abrirArchivo(fdCliente, socketFS);
		break;

	case accionCerrarArchivo:
		cerrarArchivo(fdCliente, socketFS);
		break;

	case accionBorrarArchivo:
		borrarArchivo(fdCliente, socketFS);
		break;

	case accionObtenerDatosArchivo:
		leerArchivo(fdCliente, socketFS);
		break;

	case accionReservarHeap:
		atenderSolicitudMemoriaDinamica();
		break;

	case accionLiberarHeap:
		atenderLiberacionMemoriaDinamica();
		break;

	case accionQuantumInterrumpido:
		sigusr1(fdCliente);
	break;

	}
}

void finalizar_programa_por_consola(int consola)
{
	int pid;
	recv(consola, &pid, sizeof(int32_t),MSG_WAITALL);

	t_proceso* proceso = buscarProcesoPorPID(pid);
	proceso->abortado = true;
	proceso->PCB->exitCode= ERROR_FIN_CONSOLA;

	void* buffer = malloc(sizeof(int32_t)*2);
	int32_t codigo = accionConsolaFinalizarErrorInstruccion;
	memcpy(buffer,&codigo,sizeof(int32_t));
	memcpy(buffer + sizeof(int32_t),&proceso->PCB->exitCode,sizeof(int32_t));
	send(proceso->ConsolaDuenio,buffer,sizeof(int32_t)*2,MSG_WAITALL);
	free(buffer);

//	int codAccion = accionDesalojarProceso;
//	send(proceso->CpuDuenio, &codAccion, sizeof(int32_t), 0);

}


void atender_accion_consola(int idMensaje, int memoria, int consola) {

	switch (idMensaje) {

	case envioScript:
		Accion_envio_script(memoria, consola, idMensaje);
		break;

	case finalizarProgramaAccion:
		finalizar_programa_por_consola(consola);
		break;
	}
}

/***************************SEMAFOROS Y COMPARTIDAS****************************************************/
void crearSemaforos() {

	int i = 0;
	char* sem = config.SEM_IDS[i];
	char* semValue = config.SEM_INIT[i];
	while (config.SEM_IDS[i] != '\0') {

		t_semaforo* semaforo = malloc(sizeof(t_semaforo));
		semaforo->valorSemaforo = atoi(semValue);
		semaforo->colaSemaforo = queue_create();
		dictionary_put(tablaSemaforos, sem, semaforo);
		i++;
		sem = config.SEM_IDS[i];
		semValue = config.SEM_INIT[i];
	}
}

void crearCompartidas() {
	int i = 0;

	while (config.SHARED_VARS[i] != '\0') {

		int* compartida = malloc(sizeof(int));
		*compartida = 7;
		dictionary_put(tablaCompartidas, config.SHARED_VARS[i], compartida);
		i++;
	}
}

void destruirCompartida(int* compartida) {
	free(compartida);
}

void destruirCompartidas() {
	dictionary_destroy_and_destroy_elements(tablaCompartidas, (void*)destruirCompartida);
}

void destruirSemaforo(t_semaforo* semaforo) {
	queue_destroy(semaforo->colaSemaforo);
	free(semaforo);
}

void destruirSemaforos() {
	dictionary_destroy_and_destroy_elements(tablaSemaforos, (void*)destruirSemaforo);
}

/*********************************PLANIFICACION***********************************************************/


void planificar()
{
		//Chequeo de New a Ready
		if(config.GRADO_MULTIPROG > queue_size(colaReady) && !queue_is_empty(colaNew))
		{
			t_proceso* proceso = (t_proceso*)queue_pop(colaNew);
			pedirMemoria(proceso);
		}


		//Chequeo de Ready a exec
		if (!queue_is_empty(colaReady) && !queue_is_empty(colaCPU)){

			ejecutarProceso((t_proceso*) queue_pop(colaReady),desencolarCPU(colaCPU));
		}


}

/********************************************INOTIFY*******************************************/
void recargarConfiguracion(char* pat) {
	t_config* configNuevo;
	/*char* pat = string_new();
	char cwd[1024]; // Variable donde voy a guardar el path absoluto hasta el /Debug
	string_append(&pat, getcwd(cwd, sizeof(cwd)));
	if (string_contains(pat, "/Debug")){
			string_append(&pat,"/kernel.cfg");
	}else{
		string_append(&pat, "/Debug/kernel.cfg");
	}*/
	configNuevo = config_create(pat);
	//free(pat);

	if (config_has_property(configNuevo, "QUANTUM_SLEEP")) {
			config.QUANTUM_SLEEP = config_get_int_value(configNuevo, "QUANTUM_SLEEP");
	}


	config_destroy(configNuevo);
}

void iniciarVigilanciaConfiguracion(){
	cambiosConfiguracion = inotify_init();
	inotify_add_watch(cambiosConfiguracion,".",IN_CLOSE_WRITE);
}

void procesarCambiosConfiguracion(){
	char buffer[EVENT_BUF_LEN];
	int length = read(cambiosConfiguracion, buffer, EVENT_BUF_LEN);
	int e = 0;
	while (e < length) {
		struct inotify_event *event =
				(struct inotify_event *) &buffer[e];
		if (event->len) {
			if (event->mask & IN_CLOSE_WRITE) {
				if (strcmp(event->name, "kernel.cfg") == 0) {
					recargarConfiguracion(pathConfiguracion);
				}
			}
		}
		e += EVENT_SIZE + event->len;
	}
}

/************************************** MAIN ****************************************************************/


int main(int argc, char *argv[]) {

	if(argc>1){

		FIFO = "FIFO";
		ROUND_ROBIN = "RR";
		identificadorProceso = 0;
		yes = 1;
		pathConfiguracion = argv[1];
		cargarConfiguracion(pathConfiguracion);
		crearLog(string_from_format("kernel_%d", getpid()), "KERNEL", 1);
		inicializarContexto();


		//Crear socket. Dejar reutilizable. Crear direccion del servidor. Bind. Listen.
		sockServ = crearSocket();
		reusarSocket(sockServ, yes);
		direccionServidor = crearDireccionServidor(config.PUERTO_KERNEL);
		bind_w(sockServ, &direccionServidor);
		listen_w(sockServ);

		//Conectar con memoria
		memoria = socket(AF_INET, SOCK_STREAM, 0);
		struct sockaddr_in direccionServ;
		direccionServ.sin_family = AF_INET;
		direccionServ.sin_port = htons(config.PUERTO_MEMORIA); // short, Ordenación de bytes de la red
		direccionServ.sin_addr.s_addr = inet_addr(config.IP_MEMORIA);
		memset(&(direccionServ.sin_zero), '\0', 8); // Poner ceros para rellenar el resto de la estructura
		connect(memoria, (struct sockaddr*) &direccionServ, sizeof(struct sockaddr));
		tamanioPag = obtener_tamanio_pagina(memoria);
		enviar_stack_size(memoria);

		//Conectar con FS
		int socketFS = socket(AF_INET, SOCK_STREAM, 0);
		struct sockaddr_in direccionServFS;
		direccionServFS.sin_family = AF_INET;
		direccionServFS.sin_port = htons(config.PUERTO_FS); // short, Ordenación de bytes de la red
		direccionServFS.sin_addr.s_addr = inet_addr(config.IP_FS);
		memset(&(direccionServFS.sin_zero), '\0', 8); // Poner ceros para rellenar el resto de la estructura
		connect(socketFS, (struct sockaddr*) &direccionServFS, sizeof(struct sockaddr));

		//Añadir listener al conjunto maestro
		FD_SET(sockServ, &master);
		FD_SET(cambiosConfiguracion, &master);
		FD_SET(cambiosConfiguracion, &configuracionCambio);
		//Mantener actualizado cual es el maxSock
		maxFd = sockServ;

		//Crear hilo para interaccion por terminal
		//pthread_t hiloInteraccionUsuario;
		//pthread_create(&hiloInteraccionUsuario, NULL,
		//		(void*) interactuar_con_usuario, NULL);

		//Abrimos hilo para escuchar al usuario desde la consola
		pthread_t hiloConsolaKernal;
		pthread_create(&hiloConsolaKernal, NULL, (void*)escucharConsolaKernel, NULL);

		//Bucle principal
		for (;;) {
			read_fds = master;
			if (select(maxFd + 1, &read_fds, NULL, NULL, NULL) == -1) { //Compruebo si algun cliente quiere interactuar.
				perror("select");
				exit(1);
			};

			for (fdCliente = 0; fdCliente <= maxFd; fdCliente++) {

				if (FD_ISSET(fdCliente, &read_fds)) { // Me fijo si tengo datos listos para leer
					if (fdCliente == sockServ) { //si entro en este "if", significa que tengo datos.

						// gestionar nuevas conexiones
						addrlen = sizeof(direccionCliente);
						if ((sockClie = accept(sockServ,(struct sockaddr*) &direccionCliente, &addrlen))
								== -1) {
								perror("accept");
						} else {
							printf("Server: nueva conexion de %s en socket %d\n",
							inet_ntoa(direccionCliente.sin_addr), sockClie);

							FD_SET(sockClie, &master); // añadir al conjunto maestro
							Colocar_en_respectivo_fdset();
						}
					} else {
						// gestionar datos de un cliente

						int idMensaje;

						if ((cantBytes = recv(fdCliente, &idMensaje, sizeof(int32_t), 0))
								<= 0) {

							if (FD_ISSET(fdCliente, &configuracionCambio)) { //EN CASO DE QUE EL MENSAJE LO HAYA ENVIADO INOTIFY
								procesarCambiosConfiguracion();
								printf("Nuevo QUANTUM_SLEEP: %d\n", config.QUANTUM_SLEEP);
							}else
							{
								conexion_de_cliente_finalizada();
							}

						} else {

							if (FD_ISSET(fdCliente, &bolsaConsolas)) { // EN CASO DE QUE EL MENSAJE LO HAYA ENVIADO UNA CONSOLA.

								atender_accion_consola(idMensaje, memoria, fdCliente);
							}
							if (FD_ISSET(fdCliente, &bolsaCpus)) { //EN CASO DE QUE EL MENSAJE LO HAYA ENVIADO UN CPU

								atender_accion_cpu(idMensaje, memoria, socketFS); //Argumentos que le paso muy probablemente cambien
							}
						}
					}
				}
			}
		//	if(planificacionDetenida == 0){
				planificar();
		//	}

		}
	}else{
		printf("Te olvidaste de pasarme el path del cfg\n");
	}
	return 0;
}
