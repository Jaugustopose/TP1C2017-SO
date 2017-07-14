#include "kernel.h"

//#include <sys/select.h>

/********************************** INICIALIZACIONES *****************************************************/

void cargarConfiguracion() {
	char* pat = string_new();
	char cwd[1024]; // Variable donde voy a guardar el path absoluto hasta el /Debug
	string_append(&pat, getcwd(cwd, sizeof(cwd)));
	if (string_contains(pat, "/Debug")){
		string_append(&pat,"/kernel.cfg");
	}else{
	string_append(&pat, "/Debug/kernel.cfg");
	}
	configKernel = config_create(pat);
	free(pat);

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
		printf("SEM_IDS: %s\n", config.SEM_IDS);
	}
	if (config_has_property(configKernel, "SEM_INIT")) {
		config.SEM_INIT = config_get_array_value(configKernel, "SEM_INIT");
		printf("SEM_INIT: %s\n", config.SEM_INIT);
	}
	if (config_has_property(configKernel, "SHARED_VARS")) {
		config.SHARED_VARS = config_get_array_value(configKernel,
				"SHARED_VARS");
		printf("SEM_INIT: %s\n", config.SHARED_VARS);
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

	send(sock, buffer, sizeof(int32_t)*2, 0);
}

void inicializarContexto() {
	listaDeProcesos = list_create();
	tablaSemaforos = dictionary_create();
	tablaCompartidas = dictionary_create();
	crearSemaforos();
	crearCompartidas();

	colaCPU = queue_create();
	colaNew = queue_create();
	colaReady = queue_create();
	colaExit = queue_create();
}

int pedido_Inicializar_Programa(int cliente, int paginas, int idProceso) {
	int codigoAccion = 1;
	pedidoSolicitudPaginas_t pedidoPaginas;
	pedidoPaginas.pid = idProceso;
	pedidoPaginas.cantidadPaginas = paginas;
	printf("Sizeof pedidoPaginas: %d\n", sizeof(pedidoPaginas));
	void* buffer = serializarMemoria(codigoAccion, &pedidoPaginas,
			sizeof(pedidoPaginas));
	send(cliente, buffer, sizeof(codigoAccion) + sizeof(pedidoPaginas), 0);
	free(buffer);
	//Reservo para recibir un int con el resultAccion
	int resultAccion;
	recv(cliente, &resultAccion, sizeof(int), 0);
	printf("inicializarPrograma resultó con código de acción: %d\n",
			resultAccion);

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

	int bytesEnviados = send(memoria, bufferParaAlmacenarEnMemoria, tamanioBufferParaMemoria,0);
	int k;
	for (k=0; k < m; k++){
		recv(memoria, &resultAccion, sizeof(resultAccion), 0);
		printf("almacenarBytes resultó con código de acción: %d\n", resultAccion);
	}

	return resultAccion;

}

void comprobarSockets(int maxSock, fd_set* read_fds) {
	if (select(maxSock + 1, &read_fds, NULL, NULL, NULL) == -1) { //Compruebo los sockets al mismo tiempo. Los NULL son para los writefds, exceptfds y el timeval.
		perror("select");
		exit(1);
	}
}

int obtener_tamanio_pagina(int memoria) {
	int valorRecibido;
	int idMensaje = 6;
	send(memoria, &idMensaje, sizeof(int32_t), 0);
	recv(memoria, &valorRecibido, sizeof(int32_t), 0);

	return valorRecibido;
}

int redondear(float numero) {
		int resultado;
		if((numero - (int)numero) !=0){
			numero++;
			resultado = (int) numero;
		}else {
			resultado = (int)numero;
		}

		printf("%d\n",resultado); /* prints !!!Hello World!!! */
		return resultado;
}

void interactuar_con_usuario() {

	printf(
				"--------------Configuración cargada exitosamente--------------\n\n");
		printf("Seleccione la opción que desee realizar:\n"
				"1) Listado de procesos del sistema\n"
				"2) Finalizar un proceso\n"
				"3) Consultar estado de un proceso\n"
				"4) Detener planificación\n");
}

void procesos_exit_code_corto_consola(int fileDescriptor, t_list* listaConProcesos) {

	//Encontrar cada proceso con proceso.ConsolaDuenio = fileDescriptor
	//Cambiarle el exit code a -6 (finalizo por desconexion de consola)
	//Llevo el proceso con exit code = 6 a la cola de exit
	for (fdCliente = 0; fdCliente < list_size(listaConProcesos); fdCliente++) {
		t_proceso* proceso = list_get(listaConProcesos, fdCliente);
		if (proceso -> ConsolaDuenio == fileDescriptor) {
			proceso -> PCB->exitCode = -6;
			queue_push(colaExit,&proceso);
		}

	}
	//Función privada dentro de este scope (para la condicion del remove)
			bool exit_code_de_proceso(t_proceso* p) {

				return (-6 == p->PCB->exitCode);
			}
	list_remove_by_condition(listaConProcesos,exit_code_de_proceso);
}

void liberar_procesos_de_cpu(int fileDescriptor, t_list* listaConProcesos) {

	//Encontrar cada proceso con proceso.CpuDuenio = filedescriptor
	//Y cambiarle el CpuDuenio a -1 (el menos 1 significa que no tiene cpu asignado)

	for (fdCliente = 0; fdCliente < list_size(listaConProcesos); fdCliente++) {
		t_proceso* proceso = list_get(listaConProcesos, fdCliente);
		if (proceso -> CpuDuenio == fileDescriptor) {
			proceso -> CpuDuenio = -1;
		}

	}

}

void Colocar_en_respectivo_fdset() {
	//Recibo identidad y coloco en la bolsa correspondiente
	recv(sockClie, &identidadCliente, sizeof(int32_t), 0);
	switch (identidadCliente) {

	case soyConsola:
		FD_SET(sockClie, &bolsaConsolas); //agrego una nueva consola a la bolsa de consolas
		printf("Se ha conectado una nueva consola \n");

		break;

	case soyCPU:
		FD_SET(sockClie, &bolsaCpus); //agrego un nuevo cpu a la bolsa de cpus
		queue_push(colaCPU, sockClie);
		break;
		printf("Se ha conectado un nuevo CPU  \n");
	}
	if (sockClie > maxFd) {
		// actualizar el máximo
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
		printf("Se desconecto cpu del socket %d", fdCliente);

		liberar_procesos_de_cpu(fdCliente, listaDeProcesos);
	}
	close(fdCliente); // Si se perdio la conexion, la cierro.
}
void enviarPCBaCPU(t_PCB* pcb, int cpu, int32_t accion)
{
	serializar_PCB(pcb, cpu, accion);
}

void Accion_envio_script(int tamanioScript, int memoria, int consola, int idMensaje) {
	if (config.GRADO_MULTIPROG > list_size(listaDeProcesos)) {
		printf("Procediendo a recibir tamaño script\n");
		recv(consola, &tamanioScript, sizeof(int32_t), 0);
		printf("Tamaño del script: %d\n", tamanioScript);

		char* buff = malloc(tamanioScript);
		//char* cadena = malloc(tamanio*sizeof(char));
		recv(fdCliente, buff, tamanioScript, 0);
		//memcpy(cadena,buff,tamanio * sizeof(char));
		printf("el valor de cadena es: %s\n", buff);
		///////////FIN DE DESERIALIZADOR///////////////
		identificadorProceso++;
		t_proceso* proceso = crearProceso(identificadorProceso, consola, (char*) buff);
		list_add(listaDeProcesos, proceso); //Agregar un proceso a esa bendita lista



		send(consola,&identificadorProceso,sizeof(int32_t),0);
		//CALCULO Y SOLICITO LAS PAGINAS QUE NECESITA EL SCRIPT//
		int paginasASolicitar = redondear((float) tamanioScript /(float) tamanioPag);
		int resultadoAccionInicializar = pedido_Inicializar_Programa(memoria,paginasASolicitar, proceso -> PCB->PID);
		if (resultadoAccionInicializar == 0) {//Depende de lo que devuelve si sale bien. (valor de EXIT_SUCCESS)

			queue_push(colaNew, proceso);
			printf("Se procede a preparar solicitud almacenar para enviar\n");
			proceso -> PCB->cantidadPaginas = paginasASolicitar;
			int resultadoAccionAlmacenar = enviarSolicitudAlmacenarBytes(memoria, proceso, buff, tamanioScript);
			if (resultadoAccionAlmacenar == 0) {
				//queue_pop(colaNew);
				queue_push(colaReady, proceso);
			}

			//EnviarPCBaCPU
			//int cpu = queue_pop(colaCPU);
			proceso->PCB->cantidadPaginas = paginasASolicitar;
			//enviarPCBaCPU(proceso->PCB, cpu, accionObtenerPCB);
		}
	}else{
		printf("El proceso no pudo ingresar a la cola de New ya que excede el grado de multiprogramacion\n");
	}

}

void atender_accion_cpu(int idMensaje, int tamanioScript, int memoria) {

	switch (idMensaje) {

	case accionSignal:

		break;

	case accionAsignarValorCompartida:
			obtenerAsignarCompartida();
		break;

	case accionObtenerValorCompartida:
			obtenerValorCompartida();
		break;

	case accionWait:

		break;

	case accionEscribir:

		break;

	case accionMoverCursor:

		break;

	case accionAbrirArchivo:

		break;

	case accionCrearArchivo:

		break;

	case accionBorrarArchivo:

		break;

	case accionObtenerDatosArchivo:

		break;

	case accionReservarHeap:
		atenderSolicitudMemoriaDinamica();
		break;

	case accionLiberarHeap:
		atenderLiberacionMemoriaDinamica();
		break;

	}
}

void atender_accion_consola(int idMensaje, int tamanioScript, int memoria, int consola) {

	switch (idMensaje) {

	case envioScript:
		Accion_envio_script(tamanioScript, memoria, consola, idMensaje);

	}
}

/***************************SEMAFOROS Y COMPARTIDAS****************************************************/
void crearSemaforos() {
	int i = 0;

	while (config.SEM_IDS[i] != '\0') {

		t_semaforo* semaforo = malloc(sizeof(t_semaforo));
		semaforo->valorSemaforo = atoi(config.SEM_INIT[i]);
		semaforo->colaSemaforo = queue_create();
		dictionary_put(tablaSemaforos, config.SEM_IDS[i], semaforo);

		i++;
	}
}

void crearCompartidas() {
	int i = 0;

	while (config.SHARED_VARS[i] != '\0') {

		int* compartida = malloc(sizeof(int));
		*compartida = 0;
		dictionary_put(tablaCompartidas, config.SHARED_VARS[i], compartida);
		i++;
	}
}

void destruirCompartida(int* compartida) {
	free(compartida);
}

void destruirCompartidas() {
	dictionary_destroy_and_destroy_elements(tablaCompartidas, destruirCompartida);
}

void destruirSemaforo(t_semaforo* semaforo) {
	queue_destroy(semaforo->colaSemaforo);
	free(semaforo);
}

void destruirSemaforos() {
	dictionary_destroy_and_destroy_elements(tablaSemaforos, destruirSemaforo);
}

void planificar()
{
	if(strcmp(config.ALGORITMO, FIFO) == 0){
		planificarFIFO();
	}
	else if(strcmp(config.ALGORITMO, ROUND_ROBIN) == 0)
	{
		planificarRR();
	}else
	{
		//TODO:MANEJAR ERROR DE HABER INGRESADO CUALQUIER COSA EN EL CONFIG
	}

}

void planificarRR()
{

}
void planificarFIFO()
{


	while (!queue_is_empty(colaReady) && !queue_is_empty(colaCPU)) {

			//limpiarColaListos();
			//limpiarColaCPU();
//
//		int codAccion = (int)accionContinuarProceso;
//		int cpu = (int)queue_pop(colaCPU);
//		void* buffer = malloc(sizeof(codAccion));
//		memcpy(buffer, &codAccion, sizeof(codAccion)); //PRIMERO EL CODIGO
//		send(cpu, buffer, sizeof(codAccion), 0);

			// Si no se vaciaron las listas entonces los primeros de ambas listas son validos
			if (!queue_is_empty(colaReady) && !queue_is_empty(colaCPU)){
				ejecutarProceso((t_proceso*) queue_pop(colaReady),(int) queue_pop(colaCPU));
			}
	}

}

/************************************** MAIN ****************************************************************/

int main(void) {
	identificadorProceso = 0;
	yes = 1;

	//Cargar configuracion
	cargarConfiguracion();

	inicializarContexto();

	//Crear socket. Dejar reutilizable. Crear direccion del servidor. Bind. Listen.
	sockServ = crearSocket();
	reusarSocket(sockServ, yes);
	direccionServidor = crearDireccionServidor(config.PUERTO_KERNEL);
	bind_w(sockServ, &direccionServidor);
	listen_w(sockServ);

	//Conectar con memoria
		int memoria = socket(AF_INET, SOCK_STREAM, 0);
		struct sockaddr_in direccionServ;
			direccionServ.sin_family = AF_INET;
			direccionServ.sin_port = htons(9030); // short, Ordenación de bytes de la red
			direccionServ.sin_addr.s_addr = inet_addr("127.0.0.1");
			memset(&(direccionServ.sin_zero), '\0', 8); // Poner ceros para rellenar el resto de la estructura
//		crearDireccionServidor(9030);
//		conectar_con_server(memoria, &direccionServidor);
			connect(memoria, (struct sockaddr*) &direccionServ, sizeof(struct sockaddr));
		tamanioPag = obtener_tamanio_pagina(memoria);
		enviar_stack_size(memoria);

	//Añadir listener al conjunto maestro
	FD_SET(sockServ, &master);

	//Mantener actualizado cual es el maxSock
	maxFd = sockServ;

	//Crear hilo para interaccion por terminal
	//pthread_t hiloInteraccionUsuario;
	//pthread_create(&hiloInteraccionUsuario, NULL,
	//		(void*) interactuar_con_usuario, NULL);

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
					if ((sockClie = accept(sockServ,
							(struct sockaddr*) &direccionCliente, &addrlen))
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
					int tamanioScript;

					if ((cantBytes = recv(fdCliente, &idMensaje, sizeof(int32_t), 0))
							<= 0) {

						conexion_de_cliente_finalizada();

					} else {

						if (FD_ISSET(fdCliente, &bolsaConsolas)) { // EN CASO DE QUE EL MENSAJE LO HAYA ENVIADO UNA CONSOLA.

							atender_accion_consola(idMensaje, tamanioScript, memoria, fdCliente);
						}
						if (FD_ISSET(fdCliente, &bolsaCpus)) { //EN CASO DE QUE EL MENSAJE LO HAYA ENVIADO UN CPU

							atender_accion_cpu(idMensaje, tamanioScript, memoria); //Argumentos que le paso muy probablemente cambien
						}
					}
				}
			}
		}


		//TODO:Hay que desarrollar la planificacion, pero por ahora es lo basico
		planificar();
	}
	return 0;
}


// RECORDATORIO PARA ENVIAR ALGO A TODOS LOS CLIENTES SEA QUIEN SEA (POR LAS DUDAS)

//						// Con los datos que me envió la consola, hago algo:
//
//						for (j = 0; j <= maxFd; j++) { // Para todos los que estan conectados
//							if (FD_ISSET(j, &master)) { // Me fijo si esta en el master
//
//							}
//						}
