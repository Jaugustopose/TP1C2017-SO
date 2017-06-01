
#include "kernel.h"

void pedido_Inicializar_Programa(int cliente, int paginas, int idProceso) {
	int codigoAccion = 1;
	pedidoSolicitudPaginas_t pedidoPaginas;
	pedidoPaginas.pid = idProceso;
	pedidoPaginas.cantidadPaginas = paginas;
	printf("Sizeof pedidoPaginas: %d\n", sizeof(pedidoPaginas));
	char* buffer = serializarMemoria(codigoAccion, &pedidoPaginas,
			sizeof(pedidoPaginas));
	printf("buffer serializado: %d\n", *buffer);
	send(cliente, buffer, sizeof(codigoAccion) + sizeof(pedidoPaginas), 0);
	free(buffer);
	//Reservo para recibir un int con el resultAccion
	int resultAccion;
	recv(cliente, &resultAccion, sizeof(int), 0);
	printf("inicializarPrograma resultó con código de acción: %d\n", resultAccion);
}

void cargarConfiguracion() {
	char* pat = string_new();
	char cwd[1024]; // Variable donde voy a guardar el path absoluto hasta el /Debug
	string_append(&pat, getcwd(cwd, sizeof(cwd)));
	string_append(&pat, "/Debug/kernel.cfg");
	t_config* configKernel = config_create(pat);
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
	if (config_has_property(configKernel, "GRADO_MULTIPROG")) {
		config.GRADO_MULTIPROG = config_get_int_value(configKernel,
				"GRADO_MULTIPROG");
		printf("GRADO_MULTIPROG: %d\n\n\n", config.GRADO_MULTIPROG);
	}

	printf("--------------Configuración cargada exitosamente--------------\n\n");
	printf("Seleccione la opción que desee realizar:\n"
			"1) Listado de procesos del sistema\n"
			"2) Finalizar un proceso\n"
			"3) Consultar estado de un proceso\n"
			"4) Detener planificación\n");

}

void comprobarSockets(int maxSock, fd_set* read_fds) {
	if (select(maxSock + 1, &read_fds, NULL, NULL, NULL) == -1) { //Compruebo los sockets al mismo tiempo. Los NULL son para los writefds, exceptfds y el timeval.
		perror("select");
		exit(1);
	}
}

void recibir_archivo(void* buffer){

	FILE* archivo =	fopen("archivo recibido.txt","w");
	if (archivo){
		fputs(buffer,archivo);
		fclose(archivo);
	}

}

int obtener_tamanio_pagina(int memoria) {
	int valorRecibido;
	int idMensaje = 6;
	send(memoria, &idMensaje, sizeof(int32_t), 0);
	recv(memoria, &valorRecibido, sizeof(int32_t), 0);

	return valorRecibido;
}

int paginas_que_ocupa(int unTamanio, int tamanioPagina){

	int paginas;
	paginas = (unTamanio / tamanioPagina);
}

void interactuar_con_usuario(){

	//Interactuo con el usuario
}

int main(void) {

	//VARIABLES

	fd_set master; // Conjunto maestro de file descriptor (Donde me voy a ir guardando todos los socket nuevos)
	fd_set read_fds; // Conjunto temporal de file descriptors para pasarle al select()
	fd_set bolsaConsolas; // Bolsa de consolas
	fd_set bolsaCpus; //Bolsa de bolsaCpus
	struct sockaddr_in direccionServidor; // Información sobre mi dirección
	struct sockaddr_in direccionCliente; // Información sobre la dirección del cliente
	int sockServ; // Socket de nueva conexion aceptada
	int sockClie; // Socket a la escucha
	int maxFd; // Numero del ultimo socket creado (maximo file descriptor)
	int yes = 1;
	int cantBytes; // La cantidad de bytes. Lo voy a usar para saber cuantos bytes me mandaron.
	int addrlen; // El tamaño de la direccion del cliente
	int identidadCliente;
	int i, j; // Variables para recorrer los sockets (mandar mensajes o detectar datos con el select)
	int identificadorProceso = 0;
	int tamanioPag;
	t_list* listaDeProcesos = list_create();
	t_queue* colaNew = queue_create();
	t_queue* colaReady = queue_create();
	t_queue* colaExec = queue_create();
	t_queue* colaExit = queue_create();
	FD_ZERO(&master); // Borro por si tienen basura adentro (capaz no hacen falta pero por las dudas)
	FD_ZERO(&read_fds);
	FD_ZERO(&bolsaConsolas);
	FD_ZERO(&bolsaCpus);

	cargarConfiguracion();

	int memoria = socket(AF_INET, SOCK_STREAM, 0);

	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr("127.0.0.1");
	direccionServidor.sin_port = htons(9030);

	conectar_con_server(memoria,&direccionServidor);
	tamanioPag = obtener_tamanio_pagina(memoria);


	//Crear socket. Dejar reutilizable. Crear direccion del servidor. Bind. Listen.
	sockServ = crearSocket();
	reusarSocket(sockServ, yes);
	direccionServidor = crearDireccionServidor(config.PUERTO_KERNEL);
	bind_w(sockServ, &direccionServidor);
	listen_w(sockServ);

	// añadir listener al conjunto maestro
	FD_SET(sockServ, &master);

	// Mantener actualizado cual es el maxSock
	maxFd = sockServ;

	pthread_t hiloInteraccionUsuario;
	pthread_create(&hiloInteraccionUsuario, NULL, (void*) interactuar_con_usuario, NULL);

	// bucle principal
	for (;;) {
		read_fds = master; // Me paso lo que tenga en el master al temporal.
		if (select(maxFd + 1, &read_fds, NULL, NULL, NULL) == -1) { //Compruebo los sockets al mismo tiempo. Los NULL son para los writefds, exceptfds y el timeval.
			perror("select");
			exit(1);
		};
		// explorar conexiones existentes en busca de datos que leer
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

						//Recibo identidad y coloco en la bolsa correspondiente


						recv(sockClie, &identidadCliente, sizeof(int), 0);
						switch (identidadCliente) {

						case soyConsola:
							FD_SET(sockClie, &bolsaConsolas); //agrego una nueva consola a la bolsa de consolas
							printf("Se ha conectado una nueva consola \n");
							t_list_con_duenio* listaProcesosDeConsola = list_create();
							listaProcesosDeConsola->duenio = sockClie;

							break;

						case soyCPU:
							FD_SET(sockClie, &bolsaCpus); //agrego un nuevo cpu a la bolsa de cpus
							break;
							printf("Se ha conectado un nuevo CPU  \n");
						}
						if (sockClie > maxFd) { // actualizar el máximo
							maxFd = sockClie;
						}

					}
				} else {
					// gestionar datos de un cliente

					//int* supuestoTamanio;
					//int* supuestoID;
					int idMensaje;
					int tamanioScript;


					if ((cantBytes = recv(i, &idMensaje, sizeof(int32_t), 0)) <= 0) {

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

						} else {
							FD_CLR(i, &bolsaCpus);
							printf("Se desconecto cpu del socket %d", i);
						}
						close(i); // Si se perdio la conexion, la cierro.

					} else {
						////////////DESERIALIZAR MENSAJE///////////////
						printf("el valor de idMensaje es: %d\n", idMensaje);

						switch(idMensaje) {

						case envioScript:
							recv(i,&tamanioScript,sizeof(int32_t),0);
							printf("el valor de tamanio es: %d\n", tamanioScript);
							char* buff = malloc(tamanioScript);
							//char* cadena = malloc(tamanio*sizeof(char));
							recv(i,buff,tamanioScript,0);
							//memcpy(cadena,buff,tamanio * sizeof(char));
							printf("el valor de cadena es: %s\n", buff);

						///////////FIN DE DESERIALIZADOR///////////////

							recibir_archivo(buff);
							free(buff);
							t_PCB pcb;
							pcb.PID = identificadorProceso++;


						//CALCULO Y SOLICITO LAS PAGINAS QUE NECESITA EL SCRIPT//

						int paginasASolicitar = redondear(tamanioScript / tamanioPag);
						pedido_Inicializar_Programa(memoria,paginasASolicitar,pcb.PID);
						pcb.contadorPrograma = 0;
						pcb.cantidadPaginas = paginasASolicitar;



						}



						// Con los datos que me envió la consola, hago algo:

						for (j = 0; j <= maxFd; j++) { // Para todos los que estan conectados
							if (FD_ISSET(j, &master)) { // Me fijo si esta en el master

								//Hago cosas en función de la bolsa en la que este.
								if (FD_ISSET(j, &bolsaConsolas)) {
									// Aca adentro va todo lo que quiero hacer si el cliente es una Consola

									puts("Hola consolas");
									puts("Cree el PCB!\n");

								} else {
									if (FD_ISSET(j, &bolsaCpus)) {
										// Aca adentro va todo lo que quiero hacer si el cliente es un CPU
										puts("Hola cpus");
										puts("Decime que queres que haga cpu!\n");

									}
								}
							}
						}
					}
				}
			}
		}
	}
	return 0;
}



