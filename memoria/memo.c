#include "memo.h"

void cargarConfigFile() {
	char* pat = string_new();
	char cwd[1024]; // Variable donde voy a guardar el path absoluto hasta el /Debug
	string_append(&pat, getcwd(cwd, sizeof(cwd)));
	string_append(&pat, "/memo.cfg");
	t_config* configMemo = config_create(pat);
	if (config_has_property(configMemo, "PUERTO_KERNEL")) {
		config.puerto_kernel = config_get_int_value(configMemo, "PUERTO_KERNEL");
		printf("config.PUERTO_KERNEL: %d\n", config.puerto_kernel);
	}
	if (config_has_property(configMemo, "IP_KERNEL")) {
		config.ip_kernel = config_get_string_value(configMemo, "IP_KERNEL");
		printf("config.IP_KERNEL: %s\n", config.ip_kernel);
	}
	if (config_has_property(configMemo, "PUERTO")) {
		config.puerto = config_get_int_value(configMemo, "PUERTO");
		printf("config.PUERTO: %d\n", config.puerto);
	}
	if (config_has_property(configMemo, "MARCOS")) {
		config.marcos = config_get_int_value(configMemo, "MARCOS");
		printf("config.MARCOS: %d\n", config.marcos);
	}
	if (config_has_property(configMemo, "MARCO_SIZE")) {
		config.marco_size = config_get_int_value(configMemo, "MARCO_SIZE");
		printf("config.MARCO_SIZE: %d\n", config.marco_size);
	}
	if (config_has_property(configMemo, "RETARDO_MEMORIA")) {
		config.retardo_memoria = config_get_int_value(configMemo, "RETARDO_MEMORIA");
		printf("config.RETARDO_MEMORIA: %d\n", config.retardo_memoria);
	}
}

void enviar_mensajes(int cliente, unsigned int length) {
	while (1) {
		char mensaje[length];
		fgets(mensaje, sizeof mensaje, stdin);
		send(cliente, mensaje, strlen(mensaje), 0);
	}
}

int conectar_con_server(int cliente,
		const struct sockaddr_in* direccionServidor) {
	return connect(cliente, (void*) &*direccionServidor,
			sizeof(*direccionServidor));
}

void recibir_mensajes_en_socket(int socket) {
	char* buf = malloc(1000);
	while (1) {
		int bytesRecibidos = recv(socket, buf, 1000, 0);
		if (bytesRecibidos < 0) {
			perror("Ha ocurrido un error al recibir un mensaje");
			exit(EXIT_FAILURE);
		} else if (bytesRecibidos == 0) {
			printf("Se terminó la conexión en el socket %d\n", socket);
			close(socket);
			exit(EXIT_FAILURE);
		} else {
			//Recibo mensaje e informo
			buf[bytesRecibidos] = '\0';
			printf("Recibí el mensaje de %i bytes: ", bytesRecibidos);
			puts(buf);
		}
	}
	free(buf);
}

void realizarDumpEstructurasDeMemoria(tablaPagina_t* tablaPaginasInvertida) {
	int i;
	puts("TABLA DE PÁGINAS INVERTIDA");
	printf("%*s||%*s||%*s\n", 9, "Marco  ", 9, "PID   ", 12, "Nro Página");
	for (i = 0; i < config.marcos; i++) {
		printf("%*d||%*d||%*d\n", 9, i, 9, tablaPaginasInvertida[i].pid, 12, tablaPaginasInvertida[i].nroPagina);
	}
}

int finalizarPrograma(int pid, tablaPagina_t* tablaPaginasInvertida) {
	int i;
	//-14 = pid no encontrado
	int retorno = -14;
	for (i = 0; i < config.marcos; i++) {
		//Chequeamos, aparte de que coincida el pid, que no sea -1 (corresponde a estructuras administrativas)
		if (tablaPaginasInvertida[i].pid == pid && tablaPaginasInvertida[i].pid != -1) {
			tablaPaginasInvertida[i].pid = -10;
			tablaPaginasInvertida[i].nroPagina = -1;
			retorno = EXIT_SUCCESS;
		}
	}
	return retorno;
}

int solicitarAsignacionPaginas(int pid, int cantPaginas, tablaPagina_t* tablaPaginasInvertida) {
	int i;
	int nroPag = -1;
	int j=0;
	int marcosLibres[cantPaginas];

	//Inicializamos array marcosLibres (podría no hacerse. Es por el warning de "unused")
	for (i = 0; i < cantPaginas; i++) {
		marcosLibres[i] = -1;
	}

	//TODO Esta sección está hecha para averiguar los nros de página que debemos
	//		otorgarle al PID.
	//		¿Es correcto o los nros de página los setea el kernel antes?
	//		Debiera ser así porque recorrer toda la tabla cada vez que se inicia
	//		Un programa es costoso (Se hace así porque puede tener asignadas las
	//		páginas no de manera contigua)

	//Recorremos la tabla y obtenemos el último nro de página asignado al proceso
	for (i = 0; i < config.marcos; ++i) {
		if (tablaPaginasInvertida[i].pid == pid && tablaPaginasInvertida[i].nroPagina > nroPag){
			nroPag = tablaPaginasInvertida[i].nroPagina;
		}
	}
	//Fin búsqueda nro de página del pid

	//TODO SEMAFORO DESDE ACÁ
	//Recorro la memoria hasta que se termine o la cantidad de marcos libres encontrados satisfaga el pedido
	//Se carga el array marcosLibres con las posiciones libres de tablaPaginasInvertida
	for (i = 0; i < config.marcos && j < cantPaginas; ++i) {
		//Si el pid es menor a -1 significa que está libre (por la inicialización)
		if (tablaPaginasInvertida[i].pid < -1) {
			marcosLibres[j] = i;
			j++;
		}
	}

	//¿Se puede satisfacer el pedido?
	if (j < cantPaginas) {
		perror("El número de páginas solicitadas supera el número de disponibles");
		return -11;
	} else {
		/* Los marcos libres que encontré previamente y guardé en el array marcosLibres
		 * los uso para asignar al pid en tablaPaginasInvertida
		 */
		for (i = 0; i < cantPaginas; i++) {
			tablaPaginasInvertida[ marcosLibres[i] ].pid = pid;
			tablaPaginasInvertida[ marcosLibres[i] ].nroPagina = ++nroPag; //incrementa el nroPag y luego asigna
		}
	}
	//TODO SEMAFORO HASTA ACÁ

	return EXIT_SUCCESS;

}

/**
 *
 * @param pid
 * @param nroPagina
 * @param offset
 * @param tamanio
 * @return char* a los bytes solicitados, o NULL si se produce algún error
 */
char* solicitarBytes(int pid, int nroPagina, int offset, int tamanio, tablaPagina_t* tablaPaginasInvertida){
	char* bytesSolicitados = NULL;
	int marco = buscarMarco(pid, nroPagina, tablaPaginasInvertida);
	printf("Marco encontrado solicitarBytes: %d\n", marco);
	if (marco == -10){
		printf("El nro de página %d para el pid %d no existe\n", pid, nroPagina);
	} else if ((offset + tamanio) > config.marco_size) {
		printf("El pedido de lectura excede el tamaño de la página\n");
	} else {
		printf("Tamaño solicitado: %d\n", tamanio);
		bytesSolicitados = malloc(tamanio);
		memcpy(bytesSolicitados, memoria + marco * config.marco_size + offset, tamanio);
	}

	return bytesSolicitados;

}

int buscarMarco(int pid, int nroPagina, tablaPagina_t* tablaPaginasInvertida) {
	int i;
	for (i=0; i<config.marcos; i++){
		if (tablaPaginasInvertida[i].pid==pid && tablaPaginasInvertida[i].nroPagina==nroPagina){
			return i;
		}
	}
	return -10;
}

int almacenarBytes(int pid, int nroPagina, int offset, int tamanio, char* buffer, tablaPagina_t* tablaPaginasInvertida) {
	//TODO SEMÁFORO DESDE ACÁ
	printf("Inicia almacenarBytes\n");
	int marcoPagina = buscarMarco(pid, nroPagina, tablaPaginasInvertida);
	printf("Marco encontrado: %d\n", marcoPagina);
	if ((offset + tamanio) > config.marco_size) {
		perror("El pedido excede el tamaño de la página");
		return -12;
	} else {
		char* destino = memoria + marcoPagina * config.marco_size + offset;
		memcpy(destino, buffer, tamanio);
		printf("El destino quedó almacenado: %s\n", destino);
	}
	//TODO SEMÁFORO HASTA ACÁ

	return EXIT_SUCCESS;
}

int inicializarPrograma(int pid, int cantPaginasSolicitadas, tablaPagina_t* tablaPaginasInvertida) {
	int i;
	int j = 0;
	int marcosLibres[cantPaginasSolicitadas];

	//Inicializamos array marcosLibres (podría no hacerse. Es por el warning de "unused")
	for (i = 0; i < cantPaginasSolicitadas; i++) {
		marcosLibres[i] = -1;
	}

	//TODO SEMAFORO DESDE ACÁ
	//Recorro la memoria hasta que se termine o la cantidad de marcos libres encontrados satisfaga el pedido
	for (i = 0; i < config.marcos && j < cantPaginasSolicitadas; ++i) {
		//Si el pid es menor a -1 significa que está libre (por la inicialización)
		if (tablaPaginasInvertida[i].pid < -1) {
			marcosLibres[j] = i;
			j++;
		}
	}

	//¿Se puede satisfacer el pedido?
	if (j < cantPaginasSolicitadas) {
		perror("El número de páginas solicitadas supera el número de disponibles");
		return -11;
	} else {
		/* Los marcos libres que encontré previamente y guardé en el array marcosLibres
		 * los uso para asignar al pid en tablaPaginasInvertida
		 */
		for (i = 0; i < cantPaginasSolicitadas; i++) {
			tablaPaginasInvertida[ marcosLibres[i] ].pid = pid;
			tablaPaginasInvertida[ marcosLibres[i] ].nroPagina = i;
		}
	}
	//TODO SEMAFORO HASTA ACÁ
	printf("Paginas asignadas con éxito\n");

	return EXIT_SUCCESS;

}

void escucharConsolaMemoria(tablaPagina_t* tablaPaginasInvertida) {
	printf("Escuchando nuevas solicitudes de consola en nuevo hilo\n");
	while (1) {
		printf("Ingrese una acción a realizar\n");
		puts("1: Configurar retardo memoria");
		puts("2: Realizar dump de Memoria cache");
		puts("3: Realizar dump del contenido de la Memoria completa");
		puts("4: Realizar dump del contenido de la Memoria para un proceso en particular");
		char accion[3];
		if (fgets(accion, sizeof(accion), stdin) == NULL) {
			printf("ERROR EN fgets !\n");
			return;
		}
		int codAccion = accion[0] - '0';
		switch (codAccion) {
			case retardo:
				printf("Codificar retardo!\n");
				break;
			case dumpCache:
				printf("Codificar dumpCache!\n");
				break;
			case dumpEstructurasDeMemoria:
				realizarDumpEstructurasDeMemoria(tablaPaginasInvertida);
				break;
			case dumpMemoriaCompleta:
				printf("Codificar dumpMemoriaCompleta!\n");
				break;
			case dumpMemoriaProceso:
				printf("Codificar dumpMemoriaProceso!\n");
				break;
			case flushCache:
				printf("Codificar flushCache!\n");
				break;
			case sizeMemoria:
				printf("Codificar sizeMemoria!\n");
				break;
			case sizePid:
				printf("Codificar sizePid!\n");
				break;
		}
	}
}

int main(void){

	//Setea config_t config
	cargarConfigFile();

	//Inicializacion tabla de páginas invertida
	tablaPagina_t tablaPaginasInvertida[config.marcos];
	tamanioTablaPagina = config.marcos * sizeof(tablaPagina_t);

	int cantMarcosOcupaTablaPaginas;
	if (tamanioTablaPagina % config.marco_size == 0) {
		cantMarcosOcupaTablaPaginas = (tamanioTablaPagina / config.marco_size);
	} else {
		cantMarcosOcupaTablaPaginas = (tamanioTablaPagina / config.marco_size) + 1;
	}
	printf("Cantidad de marcos que ocupa la tabla de páginas invertida: %d\n", cantMarcosOcupaTablaPaginas);

	int i;
	for (i = 0; i < config.marcos; ++i) {
		//Si son los primeros espacios, ya está ocupada por la tabla de páginas invertida
		if (i < cantMarcosOcupaTablaPaginas) {
			tablaPaginasInvertida[i].pid = -1;
			tablaPaginasInvertida[i].nroPagina = i;
		} else {
			tablaPaginasInvertida[i].pid = -10;
			tablaPaginasInvertida[i].nroPagina = -1;
		}
	}
	printf("Tabla de páginas invertida inicializada\n");
	//Fin inicialización tabla de páginas invertida

	//Inicialización memoria
	tamanioMemoria = config.marco_size * config.marcos;
	printf("Inicializando la memoria de %d bytes\n", tamanioMemoria);
	memoria = malloc(tamanioMemoria);
	memcpy(memoria, tablaPaginasInvertida, tamanioTablaPagina);
	//Fin inicialización memoria
	printf("Memoria inicializada\n");

	struct sockaddr_in direccionServidor; // Información sobre mi dirección
	struct sockaddr_in direccionCliente; // Información sobre la dirección del cliente
	int addrlen; // El tamaño de la direccion del cliente
	int sockServ; // Socket de nueva conexion aceptada
	int sockClie; // Socket a la escucha
	int cantBytesRecibidos;

	//Crear socket. Dejar reutilizable. Crear direccion del servidor. Bind. Listen.
	sockServ = crearSocket();
	reusarSocket(sockServ, 1);
	direccionServidor = crearDireccionServidor(config.puerto);
	bind_w(sockServ, &direccionServidor);
	listen_w(sockServ);
	printf("Escuchando nuevas solicitudes tcp en el puerto %d...\n", config.puerto);
	pthread_t unHilo;
	pthread_create(&unHilo, NULL, (void*) escucharConsolaMemoria, (void*) tablaPaginasInvertida);

	// gestionar nuevas conexiones
	addrlen = sizeof(direccionCliente);
	for (;;) {
		if ((sockClie = accept(sockServ, (struct sockaddr*) &direccionCliente, &addrlen)) == -1) {
			perror("Error en el accept");
		} else {
			printf("Server: nueva conexion de %s en socket %d\n", inet_ntoa(direccionCliente.sin_addr), sockClie);
			for (;;) {
				// Gestionar datos de un cliente. Recibimos el código de acción que quiere realizar.
				int codAccion;
				if ((cantBytesRecibidos = recv(sockClie, &codAccion, sizeof(int), 0)) <= 0) {
					// error o conexión cerrada por el cliente
					if (cantBytesRecibidos == 0) {
						// conexión cerrada
						printf("Server: socket %d termino la conexion\n", sockClie);
						close(sockClie);
						break;
					} else {
						perror("Se ha producido un error en el Recv");
						break;
					}
				} else {
					printf("He recibido %d bytes con la acción: %d\n", cantBytesRecibidos, codAccion);
					pedidoSolicitudPaginas_t pedidoPaginas;
					pedidoBytesMemoria_t pedidoBytes;
					pedidoAlmacenarBytesMemoria_t pedidoAlmacenarBytes;
					char* bytesAEscribir;
					char* bytesSolicitados;
					int resultAccion;
					int pidAFinalizar;

					switch (codAccion) {

					case inicializarProgramaAccion:
						recv(sockClie, &pedidoPaginas, sizeof(pedidoPaginas), 0);
						printf("Recibida solicitud de %d páginas para el pid %d\n", pedidoPaginas.cantidadPaginas, pedidoPaginas.pid);
						printf("Se procede a inicializar programa\n");
						resultAccion = inicializarPrograma(pedidoPaginas.pid,
								pedidoPaginas.cantidadPaginas,
								tablaPaginasInvertida);
						printf("Inicializar programa en Memoria terminó con resultado de acción: %d\n", resultAccion);
						send(sockClie, &resultAccion, sizeof(resultAccion), 0);
						break;

					case solicitarPaginasAccion:
						recv(sockClie, &pedidoPaginas, sizeof(pedidoPaginas), 0);
						printf("Recibida solicitud de %d páginas para el pid %d\n", pedidoPaginas.cantidadPaginas, pedidoPaginas.pid);
						printf("Se procede a solicitar páginas del programa\n");
						resultAccion = solicitarAsignacionPaginas(
								pedidoPaginas.pid,
								pedidoPaginas.cantidadPaginas,
								tablaPaginasInvertida);
						printf("Solicitar páginas adicionales terminó con resultado de acción: %d\n", resultAccion);
						send(sockClie, &resultAccion, sizeof(resultAccion), 0);
						break;

					case almacenarBytesAccion:
						recv(sockClie, &pedidoAlmacenarBytes.pedidoBytes, sizeof(pedidoAlmacenarBytes.pedidoBytes), 0);
						bytesAEscribir = malloc(pedidoAlmacenarBytes.pedidoBytes.tamanio);
						recv(sockClie, bytesAEscribir, pedidoAlmacenarBytes.pedidoBytes.tamanio, 0);
						pedidoAlmacenarBytes.buffer = bytesAEscribir;
						printf("Recibida solicitud de almacenar %d bytes para el pid %d en su página %d\n con un offset de %d\n",
								pedidoAlmacenarBytes.pedidoBytes.tamanio,
								pedidoAlmacenarBytes.pedidoBytes.pid,
								pedidoAlmacenarBytes.pedidoBytes.nroPagina,
								pedidoAlmacenarBytes.pedidoBytes.offset);
						printf("Se procede a almacenar bytes: %s\n", pedidoAlmacenarBytes.buffer);
						resultAccion = almacenarBytes(
								pedidoAlmacenarBytes.pedidoBytes.pid,
								pedidoAlmacenarBytes.pedidoBytes.nroPagina,
								pedidoAlmacenarBytes.pedidoBytes.offset,
								pedidoAlmacenarBytes.pedidoBytes.tamanio,
								pedidoAlmacenarBytes.buffer,
								tablaPaginasInvertida);
						printf("Solicitud de almacenar bytes terminó con resultado de acción: %d\n", resultAccion);
						send(sockClie, &resultAccion, sizeof(resultAccion), 0);
						free(bytesAEscribir);
						break;

					case solicitarBytesAccion:
						recv(sockClie, &pedidoBytes, sizeof(pedidoBytes), 0);
						printf("Recibida solicitud de %d bytes para el pid %d en su página %d\n con un offset de %d\n",
								pedidoAlmacenarBytes.pedidoBytes.tamanio,
								pedidoAlmacenarBytes.pedidoBytes.pid,
								pedidoAlmacenarBytes.pedidoBytes.nroPagina,
								pedidoAlmacenarBytes.pedidoBytes.offset);
						printf("Se procede a solicitar bytes\n");
						bytesSolicitados = solicitarBytes(pedidoBytes.pid,
								pedidoBytes.nroPagina, pedidoBytes.offset,
								pedidoBytes.tamanio, tablaPaginasInvertida);
						printf("Solicitar bytes devolvió los Bytes solicitados: %s\n", bytesSolicitados);
						send(sockClie, bytesSolicitados, pedidoBytes.tamanio, 0);
						free(bytesSolicitados);
						break;

					case finalizarProgramaAccion:
						recv(sockClie, &pidAFinalizar, sizeof(pidAFinalizar), 0);
						printf("Recibida solicitud para finalizar programa con pid = %d\n", pidAFinalizar);
						printf("Se procede a finalizar el programa\n");
						resultAccion = finalizarPrograma(pidAFinalizar, tablaPaginasInvertida);
						printf("Solicitud de finalizar programa terminó con resultado de acción: %d\n", resultAccion);
						send(sockClie, &resultAccion, sizeof(resultAccion), 0);
						break;

					default:
						printf("No reconozco el código de acción\n");
						resultAccion = -13;
						send(sockClie, &resultAccion, sizeof(resultAccion), 0);
					}
					printf("Fin atención acción\n");
				}
			}
		}
	}


}
