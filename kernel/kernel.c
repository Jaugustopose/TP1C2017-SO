
#include "kernel.h"

int pedido_Inicializar_Programa(int cliente, int paginas, int idProceso) {
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

	return resultAccion;
}

void enviarSolicitudAlmacenarBytes(int cliente,t_proceso unProceso, char* buffer,int tamanioTotal) {
	int codigoAccion = 3;
	char* buffer2;
	int m;
	for (m=0; m <= unProceso.PCB.cantidadPaginas; m++)
	if (tamanioTotal > tamanioPag) {

		tamanioTotal = tamanioTotal - tamanioPag;


	}
	pedidoAlmacenarBytesMemoria_t pedidoAlmacenar;
	pedidoAlmacenar.pedidoBytes.pid = unProceso.PCB.PID;
	pedidoAlmacenar.pedidoBytes.nroPagina =
	pedidoAlmacenar.pedidoBytes.offset = 0;
	pedidoAlmacenar.pedidoBytes.tamanio =
	pedidoAlmacenar.buffer = buffer;
	printf("pedidoAlmacenar.buffer: %s\n", pedidoAlmacenar.buffer);

	//No paso sizeof(pedidoAlmacenar) porque el último campo es un puntero y me dará siempre 4 bytes. Entonces lo armo en dos partes,
	// y en la segunda el tamaño es el que indica pedidoAlmacenar.pedidoBytes.tamanio.
	// Luego, en el send, armo la suma que me da la cantidad de bytes correctos (para que no tome el puntero, sino el verdadero tamaño
	buffer2 = serializarMemoria(codigoAccion, &pedidoAlmacenar.pedidoBytes, sizeof(pedidoAlmacenar.pedidoBytes));
	memcpy(buffer2 + sizeof(codigoAccion) + sizeof(pedidoAlmacenar.pedidoBytes), pedidoAlmacenar.buffer, pedidoAlmacenar.pedidoBytes.tamanio);
	printf("luego de serializar: %s\n", buffer2 + sizeof(codigoAccion) + sizeof(pedidoAlmacenar.pedidoBytes));

	send(cliente, buffer2, sizeof(codigoAccion) + sizeof(pedidoAlmacenar.pedidoBytes) + pedidoAlmacenar.pedidoBytes.tamanio, 0);

	//Ahora recibo la respuesta
	int resultAccion;
	recv(cliente, &resultAccion, sizeof(resultAccion), 0);
	printf("almacenarBytes resultó con código de acción: %d\n", resultAccion);

//	free(buffer2);
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

void procesos_exit_code_a_cero(int fileDescriptor, t_list* listaConProcesos){

	//Encontrar cada proceso con proceso.ConsolaDuenio = fileDescriptor
	// Y cambiarle el exit code a -6 (finalizo por desconexion de consola)

}

int main(void) {

	cargarConfiguracion();
	listaDeProcesos = list_create();


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

							procesos_exit_code_a_cero(i,listaDeProcesos); //TODO

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
							t_proceso proceso;
							proceso.PCB.PID = identificadorProceso;
							proceso.ConsolaDuenio = i;
							proceso.CpuDuenio = -1;
							list_add(listaDeProcesos,&proceso); //TODO: Agregar un proceso a esa bendita lista

							identificadorProceso++;


						//CALCULO Y SOLICITO LAS PAGINAS QUE NECESITA EL SCRIPT//

						int paginasASolicitar = redondear(tamanioScript / tamanioPag);
						int resultadoAccion = pedido_Inicializar_Programa(memoria,paginasASolicitar,proceso.PCB.PID);

						if(resultadoAccion = 0) { //Depende de lo que devuelve si sale bien. (valor de EXIT_SUCCESS)

							proceso.PCB.contadorPrograma = 0;
							proceso.PCB.cantidadPaginas = paginasASolicitar;

							int k;
							for(k=0; k <= proceso.PCB.cantidadPaginas;k++){
								enviarSolicitudAlmacenarBytes(memoria,proceso,buff,tamanioScript);
							}

						}




						}



						// Con los datos que me envió la consola, hago algo:

						for (j = 0; j <= maxFd; j++) { // Para todos los que estan conectados
							if (FD_ISSET(j, &master)) { // Me fijo si esta en el master

								//Hago cosas en función de la bolsa en la que este.
								if (FD_ISSET(j, &bolsaConsolas)) {
									// Aca adentro va lo que quiero hacer si el cliente es una Consola

									puts("Hola consolas");
									puts("Cree el PCB!\n");

								} else {
									if (FD_ISSET(j, &bolsaCpus)) {
										// Aca adentro va lo que quiero hacer si el cliente es un CPU
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



