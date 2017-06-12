#include "kernel.h"

/********************************** INICIALIZACIONES *****************************************************/

void cargarConfiguracion() {
	char* pat = string_new();
	char cwd[1024]; // Variable donde voy a guardar el path absoluto hasta el /Debug
	string_append(&pat, getcwd(cwd, sizeof(cwd)));
	string_append(&pat, "/Debug/kernel.cfg");
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
		config.PUERTO_CONSOLA = config_get_int_value(configKernel,"PUERTO_CONSOLA");
		printf("PUERTO_CONSOLA: %d\n", config.PUERTO_CONSOLA);
	}
	if (config_has_property(configKernel, "GRADO_MULTIPROG")) {
		config.GRADO_MULTIPROG = config_get_int_value(configKernel,"GRADO_MULTIPROG");
		printf("GRADO_MULTIPROG: %d\n\n\n", config.GRADO_MULTIPROG);
	}
	if (config_has_property(configKernel, "SEM_IDS")) {
		config.SEM_IDS = config_get_array_value(configKernel,"SEM_IDS");
		printf("SEM_IDS: %d\n\n\n", config.SEM_IDS);
	}
	if (config_has_property(configKernel, "SEM_INIT")) {
		config.SEM_INIT = config_get_array_value(configKernel,"SEM_INIT");
		printf("SEM_INIT: %d\n\n\n", config.SEM_INIT);
	}
	if (config_has_property(configKernel, "SHARED_VARS")) {
		config.SHARED_VARS = config_get_array_value(configKernel,"SHARED_VARS");
		printf("SEM_INIT: %d\n\n\n", config.SHARED_VARS);
	}


	printf(
			"--------------Configuración cargada exitosamente--------------\n\n");
	printf("Seleccione la opción que desee realizar:\n"
			"1) Listado de procesos del sistema\n"
			"2) Finalizar un proceso\n"
			"3) Consultar estado de un proceso\n"
			"4) Detener planificación\n");

}

void inicializarContexto()
{
	listaDeProcesos = list_create();
	tablaSemaforos = dictionary_create();
	tablaCompartidas = dictionary_create();
	crearSemaforos();
	crearCompartidas();
}

int pedido_Inicializar_Programa(int cliente, int paginas, int idProceso) {
	int codigoAccion = 1;
	pedidoSolicitudPaginas_t pedidoPaginas;
	pedidoPaginas.pid = idProceso;
	pedidoPaginas.cantidadPaginas = paginas;
	printf("Sizeof pedidoPaginas: %d\n", sizeof(pedidoPaginas));
	void* buffer = serializarMemoria(codigoAccion, &pedidoPaginas,
			sizeof(pedidoPaginas));
	printf("buffer serializado: %d\n", *buffer);
	send(cliente, buffer, sizeof(codigoAccion) + sizeof(pedidoPaginas), 0);
	free(buffer);
	//Reservo para recibir un int con el resultAccion
	int resultAccion;
	recv(cliente, &resultAccion, sizeof(int), 0);
	printf("inicializarPrograma resultó con código de acción: %d\n",
			resultAccion);

	return resultAccion;
}

void enviarSolicitudAlmacenarBytes(int cliente, t_proceso unProceso, char* buffer, int tamanioTotal) {
	int codigoAccion = 3;
	char* buffer2;
	int m;
	int tamanioAAlmacenar;
	//Armo buffer para enviar
	void* bufferParaAlmacenarEnMemoria = malloc(
			(sizeof(codigoAccion) + sizeof(unProceso.PCB.PID) + sizeof(int32_t)
					+ sizeof(int32_t)) * unProceso.PCB.cantidadPaginas
					+ tamanioTotal);

	for (m = 0; m < unProceso.PCB.cantidadPaginas; m++) {
		if (tamanioTotal > tamanioPag) {

			tamanioAAlmacenar = tamanioPag;
			tamanioTotal = tamanioTotal - tamanioPag;
		} else {
			tamanioAAlmacenar = tamanioTotal;
		}
		pedidoAlmacenarBytesMemoria_t pedidoAlmacenar;
		pedidoAlmacenar.pedidoBytes.pid = unProceso.PCB.PID;
		pedidoAlmacenar.pedidoBytes.nroPagina = m;
		pedidoAlmacenar.pedidoBytes.offset = 0;
		pedidoAlmacenar.pedidoBytes.tamanio = tamanioAAlmacenar;
		pedidoAlmacenar.buffer = buffer;
		void* buffer2 = serializarMemoria(codigoAccion, pedidoAlmacenar.buffer,
				pedidoAlmacenar.pedidoBytes.tamanio);
		int tamanioBuffer2 = sizeof(pedidoAlmacenar.pedidoBytes)
							 + sizeof(pedidoAlmacenar.pedidoBytes.tamanio)
							 + sizeof(codigoAccion);
		if (m != 0) {
			memcpy(
					bufferParaAlmacenarEnMemoria
					+ ((sizeof(codigoAccion) + sizeof(pedidoAlmacenar.pedidoBytes)) * m) + tamanioPag,
					buffer2, tamanioBuffer2);
		} else {
			memcpy(bufferParaAlmacenarEnMemoria, buffer2, tamanioBuffer2);
		}
		free(buffer2);
		send(cliente, bufferParaAlmacenarEnMemoria,
			 sizeof(codigoAccion) + sizeof(pedidoAlmacenar.pedidoBytes)
			 + pedidoAlmacenar.pedidoBytes.tamanio, 0);

		//Ahora recibo la respuesta
		int resultAccion;
		recv(cliente, &resultAccion, sizeof(resultAccion), 0);
		printf("almacenarBytes resultó con código de acción: %d\n",
				resultAccion);

			free(buffer2);
	}
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

int paginas_que_ocupa(int unTamanio, int tamanioPagina) {

	int paginas;
	paginas = (unTamanio / tamanioPagina);
}

void interactuar_con_usuario() {

	//Interactuo con el usuario
}

void procesos_exit_code_a_cero(int fileDescriptor, t_list* listaConProcesos) {

	//Encontrar cada proceso con proceso.ConsolaDuenio = fileDescriptor
	// Y cambiarle el exit code a -6 (finalizo por desconexion de consola)

}

void Colocar_en_respectivo_fdset() {
	//Recibo identidad y coloco en la bolsa correspondiente
	recv(sockClie, &identidadCliente, sizeof(int), 0);
	switch (identidadCliente) {

	case soyConsola:
		FD_SET(sockClie, &bolsaConsolas); //agrego una nueva consola a la bolsa de consolas
		printf("Se ha conectado una nueva consola \n");

		break;

	case soyCPU:
		FD_SET(sockClie, &bolsaCpus); //agrego un nuevo cpu a la bolsa de cpus
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
		printf("Server: socket %d termino la conexion\n", i);
	} else {
		perror("recv");
	}
	// Eliminar del conjunto maestro y su respectiva bolsa
	FD_CLR(i, &master);
	if (FD_ISSET(i, &bolsaConsolas)) {
		FD_CLR(i, &bolsaConsolas);
		printf("Se desconecto consola del socket %d", i);

		procesos_exit_code_a_cero(i, listaDeProcesos); //TODO

	} else {
		FD_CLR(i, &bolsaCpus);
		printf("Se desconecto cpu del socket %d", i);

		liberar_procesos_de_consola(i, listaDeProcesos); //TODO
	}
	close(i); // Si se perdio la conexion, la cierro.
}

void Accion_envio_script(int tamanioScript, int memoria) {
	recv(i, &tamanioScript, sizeof(int32_t), 0);
	printf("el valor de tamanio es: %d\n", tamanioScript);
	char* buff = malloc(tamanioScript);
	//char* cadena = malloc(tamanio*sizeof(char));
	recv(i, buff, tamanioScript, 0);
	//memcpy(cadena,buff,tamanio * sizeof(char));
	printf("el valor de cadena es: %s\n", buff);
	///////////FIN DE DESERIALIZADOR///////////////
	t_proceso proceso = crearProceso(identificadorProceso, i, (char*) buff);
	list_add(listaDeProcesos, &proceso); //TODO: Agregar un proceso a esa bendita lista
	identificadorProceso++;
	//CALCULO Y SOLICITO LAS PAGINAS QUE NECESITA EL SCRIPT//
	int paginasASolicitar = redondear(tamanioScript / tamanioPag);
	int resultadoAccion = pedido_Inicializar_Programa(memoria,
			paginasASolicitar, proceso.PCB.PID);
	if (resultadoAccion == 0) {
		//Depende de lo que devuelve si sale bien. (valor de EXIT_SUCCESS)
		proceso.PCB.cantidadPaginas = paginasASolicitar;
		enviarSolicitudAlmacenarBytes(memoria, proceso, buff, tamanioScript);
	}
}

void atender_accion_cpu(int idMensaje, int tamanioScript, int memoria) {

	switch (idMensaje) {

	case accionSignal:

	break;

	case accionAsignarValorCompartida:

	break;

	case accionObtenerValorCompartida:

	break;

	case accionWait:

	break;

	case accionSignal:

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

	break;

	case accionLiberarHeap:

	break;

	}
}

void atender_accion_consola(int idMensaje, int tamanioScript, int memoria) {

	switch (idMensaje) {
	
	case envioScript:
		Accion_envio_script(tamanioScript, memoria);

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
	dictionary_destroy_and_destroy_elements(tablaCompartidas, (void*)destruirSemaforo);
}

void destruirSemaforo(t_semaforo* semaforo) {
	queue_destroy(semaforo->colaSemaforo);
	free(semaforo);
}

void destruirSemaforos() {
	dictionary_destroy_and_destroy_elements(tablaSemaforos, (void*)destruirSemaforo);
}

/************************************** MAIN ****************************************************************/

int main(void) {
	//Cargar configuracion
	cargarConfiguracion();

	inicializarContexto();

	//Conectar con memoria
	int memoria = socket(AF_INET, SOCK_STREAM, 0);
	crearDireccionServidor(9030);
	conectar_con_server(memoria, &direccionServidor);
	tamanioPag = obtener_tamanio_pagina(memoria);

	//Crear socket. Dejar reutilizable. Crear direccion del servidor. Bind. Listen.
	sockServ = crearSocket();
	reusarSocket(sockServ, yes);
	direccionServidor = crearDireccionServidor(config.PUERTO_KERNEL);
	bind_w(sockServ, &direccionServidor);
	listen_w(sockServ);

	//Añadir listener al conjunto maestro
	FD_SET(sockServ, &master);

	//Mantener actualizado cual es el maxSock
	maxFd = sockServ;

	//Crear hilo para interaccion por terminal
	pthread_t hiloInteraccionUsuario;
	pthread_create(&hiloInteraccionUsuario, NULL,
			(void*) interactuar_con_usuario, NULL);

	//Bucle principal
	for (;;) {
		read_fds = master;
		if (select(maxFd + 1, &read_fds, NULL, NULL, NULL) == -1) { //Compruebo si algun cliente quiere interactuar.
			perror("select");
			exit(1);
		};

		for (i = 0; i <= maxFd; i++) {
			if (FD_ISSET(i, &read_fds)) { // Me fijo si tengo datos listos para leer
				if (i == sockServ) { //si entro en este "if", significa que tengo datos.

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

					if ((cantBytes = recv(i, &idMensaje, sizeof(int32_t), 0))
							<= 0) {

						conexion_de_cliente_finalizada();

					} else {

						if (FD_SET(i,&bolsaConsolas)){ // EN CASO DE QUE EL MENSAJE LO HAYA ENVIADO UNA CONSOLA.

							atender_accion_consola(idMensaje, tamanioScript, memoria);
						}
						if (FD_SET(i,&bolsaCpus)) { //EN CASO DE QUE EL MENSAJE LO HAYA ENVIADO UN CPU

							atender_accion_cpu(idMensaje,tamanioScript,memoria); //Argumentos que le paso muy probablemente cambien
						}
					}
				}
			}
		}
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



