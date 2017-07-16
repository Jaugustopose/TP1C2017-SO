#include "memo.h"

void cargarConfigFile() {
	char* pat = string_new();
	char cwd[1024]; // Variable donde voy a guardar el path absoluto hasta el /Debug
	string_append(&pat, getcwd(cwd, sizeof(cwd)));
	if (string_contains(pat, "/Debug")) {
		string_append(&pat, "/memo.cfg");
	} else {
		string_append(&pat, "/Debug/memo.cfg");
	}

	t_config* configMemo = config_create(pat);
	if (config_has_property(configMemo, "PUERTO_KERNEL")) {
		config.puerto_kernel = config_get_int_value(configMemo,
				"PUERTO_KERNEL");
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
		config.retardo_memoria = config_get_int_value(configMemo,
				"RETARDO_MEMORIA");
		printf("config.RETARDO_MEMORIA: %d\n", config.retardo_memoria);
	}
}

/* Inicialización vector overflow. Cada posición tiene una lista enlazada que guarda números de frames.
 * Se llenará a medida que haya colisiones correspondientes a esa posición del vector. */
void inicializarOverflow(int cantidad_de_marcos) {
	overflow = malloc(sizeof(t_list*) * cantidad_de_marcos);
	int i;
	for (i = 0; i < cantidad_de_marcos; ++i) { /* Una lista por frame */
		overflow[i] = list_create();
	}
}

/* Función Hash */
unsigned int calcularPosicion(int pid, int num_pagina) {
	char str1[20];
	char str2[20];
	int ultimo_marco = config.marcos - 1;
	sprintf(str1, "%d", pid);
	sprintf(str2, "%d", num_pagina);
	strcat(str1, str2);
	//Función módulo mas corrimiento por la cant. de marcos que ocupa la tabla de pag invertida
	unsigned int indice = atoi(str1) % config.marcos + cantMarcosOcupaTablaPaginas;

	/*Si al sumar los marcos que ocupa la tabla de páginas invertida me excedo,
	 * recalculo la posición
	 */
	if (indice > ultimo_marco) {
		indice = indice - ultimo_marco + cantMarcosOcupaTablaPaginas;
	}
	return indice;
}

/* En caso de colisión, busca el siguiente frame en el vector de overflow.
 * Retorna el número de frame donde se encuentra la página. */
int32_t buscarEnOverflow(int32_t indice, int32_t pid, int32_t pagina, tablaPagina_t* tablaPaginasInvertida) {
	int32_t i = 0;
	int32_t frame = -10;
	for (i = 0; i < list_size(overflow[indice]); i++) {
		if (esMarcoCorrecto((int32_t)list_get(overflow[indice], i), pid, pagina, tablaPaginasInvertida)) {
			frame = (int32_t)list_get(overflow[indice], i);
		}
	}
	return frame;
}

/* Si en la posición candidata de la tabla de páginas invertida se encuentran el pid y páginas recibidos
 * por parámetro, se trata del marco correcto */
int esMarcoCorrecto(int pos_candidata, int pid, int pagina, tablaPagina_t* tablaPaginasInvertida) {

	return tablaPaginasInvertida[pos_candidata].pid == pid && tablaPaginasInvertida[pos_candidata].nroPagina == pagina;

}

/* Agrega una entrada a la lista enlazada correspondiente a una posición del vector de overflow */
void agregarSiguienteEnOverflow(int pos_inicial, int nro_frame) {
	list_add(overflow[pos_inicial], nro_frame);
}

/* Elimina un frame de la lista enlazada correspondiente a una determinada posición del vector de overflow  */
void borrarDeOverflow(int posicion, int frame) {
	int i = 0;
	int index_frame;

	for (i = 0; i < list_size(overflow[posicion]); i++) {
		if (frame == (int) list_get(overflow[posicion], i)) {
			index_frame = i;
			i = list_size(overflow[posicion]);
		}
	}

	list_remove(overflow[posicion], index_frame);
}

void realizarDumpEstructurasDeMemoria(tablaPagina_t* tablaPaginasInvertida) {
	int i;
	t_list* listaProcesosActivos = list_create();

	puts("TABLA DE PÁGINAS INVERTIDA");
	printf("%*s||%*s||%*s\n", 9, "Marco  ", 9, "PID   ", 12, "Nro Página");
	for (i = 0; i < config.marcos; i++) {
		printf("%*d||%*d||%*d\n", 9, i, 9, tablaPaginasInvertida[i].pid, 12,
				tablaPaginasInvertida[i].nroPagina);
		//Función privada dentro de este scope (para el closure del find)
		int _soy_pid_buscado(void *p) {
			return p == tablaPaginasInvertida[i].pid;
		}
		//Si no lo encontramos en la lista de activos lo agregamos
		if (list_find(listaProcesosActivos, _soy_pid_buscado) == NULL) {
			list_add(listaProcesosActivos, tablaPaginasInvertida[i].pid);
		}

	}
	puts("LISTADO DE PROCESOS ACTIVOS");
	for (i = 0; i < list_size(listaProcesosActivos); i++) {
		int pid = (int) list_get(listaProcesosActivos, i);
		//No imprimimos los nros de pid correspondientes a estructuras administrativas (-1) ni libres (-10)
		if (pid >= 0) {
			printf("PID: %d\n", pid);
		}
	}
	puts("");
	list_destroy(listaProcesosActivos);
}

void realizarDumpContenidoMemoriaCompleta(tablaPagina_t* tablaPaginasInvertida) {
	int i;
	char* bufferPagina = malloc(config.marco_size);
	printf("Se procede a imprimir por pantalla el contenido de cada marco:\n");
	for (i = 0; i < config.marcos; i++) {
		memcpy(bufferPagina, memoria + i * config.marco_size,
				config.marco_size);
		printf("Marco: %d, pid: %d, pag: %d, contenido: %s\n", i,
				tablaPaginasInvertida[i].pid,
				tablaPaginasInvertida[i].nroPagina, bufferPagina);
	}
	free(bufferPagina);
}

int finalizarPrograma(int pid, tablaPagina_t* tablaPaginasInvertida) {
	int i;
	//-14 = pid no encontrado
	int retorno = -14;
	for (i = 0; i < config.marcos; i++) {
		//Chequeamos, aparte de que coincida el pid, que no sea -1 (corresponde a estructuras administrativas)
		if (tablaPaginasInvertida[i].pid == pid
				&& tablaPaginasInvertida[i].pid != -1) {
			tablaPaginasInvertida[i].pid = -10;
			tablaPaginasInvertida[i].nroPagina = -1;
			retorno = EXIT_SUCCESS;
		}
	}

	return retorno;
}

int liberarPaginaPid(int pid, int nroPagina, tablaPagina_t* tablaPaginasInvertida) {

	int resultAccion;

	int marco = buscarMarco(pid, nroPagina, tablaPaginasInvertida);
	printf("Marco encontrado solicitarBytes: %d\n", marco);
	if (marco == -10) {
		printf("El nro de página %d para el pid %d no existe\n", pid, nroPagina);
		resultAccion = marco;
	} else {
		//Liberamos Marco
		tablaPaginasInvertida[marco].pid = -10;
		tablaPaginasInvertida[marco].nroPagina = -1;
		resultAccion = EXIT_SUCCESS;
	}

	return resultAccion;
}

int solicitarAsignacionPaginas(int pid, int cantPaginas, tablaPagina_t* tablaPaginasInvertida) {
	int i;
	int nroPag = -1, nroPagUltimo = -1;
	int cantidadPosicionesEncontradas=0;
	//Matriz de marcos libres: Primera columna corresponde al marco candidato que corresponderia según función de hash
	//						   Segunda columna corresponde al verdadero marco libre que se encontró libre en el reintento.
	int marcosLibres[cantPaginas][2];

	//Inicializamos matriz marcosLibres
	for (i = 0; i < cantPaginas; i++) {
		marcosLibres[i][0] = -1;
		marcosLibres[i][1] = -1;
	}

	//TODO Esta sección está hecha para averiguar los nros de página que debemos
	//		otorgarle al PID.
	//		¿Es correcto o los nros de página los setea el kernel antes?
	//		Debiera ser así porque recorrer toda la tabla cada vez que se inicia
	//		Un programa es costoso (Se hace así porque puede tener asignadas las
	//		páginas no de manera contigua)

	//Recorremos la tabla y obtenemos el último nro de página asignado al proceso
	for (i = 0; i < config.marcos; ++i) {
		if (tablaPaginasInvertida[i].pid == pid && tablaPaginasInvertida[i].nroPagina > nroPagUltimo){
			nroPagUltimo = tablaPaginasInvertida[i].nroPagina;
		}
	}
	printf("Último nro de página encontrado para pid %d es: %d\n", pid, nroPagUltimo);
	//Fin búsqueda nro de página del pid
	nroPag = nroPagUltimo;
	//TODO SEMAFORO DESDE ACÁ
	//Recorro la memoria hasta que se termine o la cantidad de marcos libres encontrados satisfaga el pedido
	for (i = 0; cantidadPosicionesEncontradas < cantPaginas; ++i) {
		nroPag++;
		int marco_candidato = calcularPosicion(pid, nroPag);

		//Si el pid es menor a -1 significa que está libre (por la inicialización)
		if (tablaPaginasInvertida[marco_candidato].pid < -1 && !estaElMarcoReservado(marco_candidato, cantPaginas, marcosLibres)) {
			marcosLibres[cantidadPosicionesEncontradas][0] = marco_candidato;
			cantidadPosicionesEncontradas++;
		} else {
			//Iterar por la memoria avanzando de a uno (+1) hasta encontrar frame libre
			int marco = marco_candidato + 1;
			//Rehash
			while((marco > marco_candidato && marco < config.marcos) || marco < marco_candidato){

				if (tablaPaginasInvertida[marco].pid < -1 && !estaElMarcoReservado(marco, cantPaginas, marcosLibres)) {
					marcosLibres[cantidadPosicionesEncontradas][0] = marco_candidato;
					marcosLibres[cantidadPosicionesEncontradas][1] = marco;
					cantidadPosicionesEncontradas++;
					break;
				}
				//Antes de incrementar el marco nos fijamos que no estemos en el final de la memoria
				//y tengamos que empezar desde el principio
				if (marco < config.marcos - 1) {
					marco++;
				} else {
					marco = cantMarcosOcupaTablaPaginas;
				}
			}

			//Si salió porque dio la vuelta y volvió al marco_candidato -> No hay más lugar en memoria
			if ( marco == marco_candidato) {
				return -11;
			}
		}
	}

	/*
	 * Los marcos libres que encontramos previamente y guardamos en la matriz marcosLibres
	 * los usamos para asignar al pid en tablaPaginasInvertida. Guardamos en su respectivo
	 * Overflow a los que hayan colisionado
	 */
	char* buffer = malloc(config.marco_size);
	memset(buffer, '\0', config.marco_size);
	for (i = 0; i < cantPaginas; i++) {
		//Si la segunda columna es -1 significa que se el marco es el devuelto por la función hash
		//Si no, el marco utilizado es el encontrado en el recorrido hecho luego de la colisión
		//También se limpia el contenido de cada marco cuando es asignado
		nroPagUltimo++;
		if ( marcosLibres[i][1] == -1) {
			tablaPaginasInvertida[ marcosLibres[i][0] ].pid = pid;
			tablaPaginasInvertida[ marcosLibres[i][0] ].nroPagina = nroPagUltimo;
			memcpy(memoria + marcosLibres[i][0] * config.marco_size, buffer, config.marco_size);
		} else {
			tablaPaginasInvertida[ marcosLibres[i][1] ].pid = pid;
			tablaPaginasInvertida[ marcosLibres[i][1] ].nroPagina = nroPagUltimo;
			agregarSiguienteEnOverflow(marcosLibres[i][0], marcosLibres[i][1]);
			memcpy(memoria + marcosLibres[i][1] * config.marco_size, buffer, config.marco_size);
		}
	}

	free(buffer);

	//TODO SEMAFORO HASTA ACÁ
	printf("Paginas asignadas con éxito\n");

	return EXIT_SUCCESS;

}

/**
 *
 * @param pid
 * @param nroPagina
 * @param offset
 * @param tamanio
 * @return char* con [codResult + bytesSolicitados]
 * codResult = int
 * bytesSolicitados = char*
 */
char* solicitarBytes(int pid, int nroPagina, int offset, int tamanio, tablaPagina_t* tablaPaginasInvertida) {
	int codResult;
	char* respuesta = malloc(tamanio + sizeof(codResult));
	char* bytesSolicitados = malloc(tamanio);
	memset(bytesSolicitados, '\0', tamanio);
	memset(respuesta, '\0', tamanio + sizeof(codResult));
	int marco = buscarMarco(pid, nroPagina, tablaPaginasInvertida);
	printf("Marco encontrado solicitarBytes: %d\n", marco);
	if (marco == -10) {
		codResult = marco;
		printf("El nro de página %d para el pid %d no existe\n", pid,
				nroPagina);
	} else if ((offset + tamanio) > config.marco_size) {
		codResult = -12;
		printf("El pedido de lectura excede el tamaño de la página\n");
	} else {
		printf("Tamaño solicitado: %d\n", tamanio);
		codResult = EXIT_SUCCESS;
		usleep(config.retardo_memoria * 1000);
		memcpy(bytesSolicitados, memoria + marco * config.marco_size + offset, tamanio);
	}
	memcpy(respuesta, &codResult, sizeof(codResult));
	memcpy(respuesta + sizeof(codResult), bytesSolicitados, tamanio);
	return respuesta;

}

int buscarMarco(int pid, int nroPagina, tablaPagina_t* tablaPaginasInvertida) {

	int marco_candidato = calcularPosicion(pid, nroPagina);
	if (esMarcoCorrecto(marco_candidato, pid, nroPagina, tablaPaginasInvertida)){
		return marco_candidato;
	} else {
		return buscarEnOverflow(marco_candidato, pid, nroPagina, tablaPaginasInvertida);
	}

}

/**
 *
 * Procesa un pedido de almacenamiento de bytes para un pid en un nro de página determinado del mismo
 * con un offset y de un tamaño determinados.
 *
 * @param pid
 * @param nroPagina
 * @param offset
 * @param tamanio
 * @param buffer
 * @param tablaPaginasInvertida
 * @return int con el resultado de la acción
 * 			  0: Éxito
 * 			-12: El pedido excede le tamaño de página
 */
int almacenarBytes(int pid, int nroPagina, int offset, int tamanio, void* buffer, tablaPagina_t* tablaPaginasInvertida) {
	//TODO SEMÁFORO DESDE ACÁ
	printf("Inicia almacenarBytes\n");
	if ((offset + tamanio) > config.marco_size) {
			printf("El pedido excede el tamaño de la página\n");
			return -12;
	} else {
		int marcoPagina = buscarMarco(pid, nroPagina, tablaPaginasInvertida);
		printf("Marco candidato: %d\n", marcoPagina);
		if (marcoPagina > -1) {
			char* destino = memoria + marcoPagina * config.marco_size + offset;
			usleep(config.retardo_memoria * 1000);
			memcpy(destino, buffer, tamanio);
			printf("El destino quedó almacenado: %s\n", destino);
		} else {
			printf("Marco no encontrado para pid y #pagina especificados\n");
			return -10;
		}
	}
	//TODO SEMÁFORO HASTA ACÁ

	return EXIT_SUCCESS;
}

int inicializarPrograma(int pid, int cantPaginasSolicitadas, tablaPagina_t* tablaPaginasInvertida) {

	int cantRealPaginasSolicitadas = cantPaginasSolicitadas + stack_size;

	int i;
	int cantPosicionesEncontradas = 0;
	//Matriz de marcos libres: Primera columna corresponde al marco candidato que corresponderia según función de hash
	//						   Segunda columna corresponde al verdadero marco libre que se encontró libre en el reintento.
	int marcosLibres[cantRealPaginasSolicitadas][2];

	//Inicializamos matriz marcosLibres
	for (i = 0; i < cantRealPaginasSolicitadas; i++) {
		marcosLibres[i][0] = -1;
		marcosLibres[i][1] = -1;
	}

	//TODO SEMAFORO DESDE ACÁ
	//Recorro la memoria hasta que se termine o la cantidad de marcos libres encontrados satisfaga el pedido
	for (i = 0; cantPosicionesEncontradas < cantRealPaginasSolicitadas; ++i) {

		int marco_candidato = calcularPosicion(pid, i);

		//Si el pid es menor a -1 significa que está libre (por la inicialización)
		if (tablaPaginasInvertida[marco_candidato].pid < -1 && !estaElMarcoReservado(marco_candidato, cantRealPaginasSolicitadas, marcosLibres)) {
			marcosLibres[cantPosicionesEncontradas][0] = marco_candidato;
			cantPosicionesEncontradas++;
		} else {
			//Iterar por la memoria avanzando de a uno (+1) hasta encontrar frame libre
			int marco = marco_candidato + 1;
			//Rehash
			while((marco > marco_candidato && marco < config.marcos) || marco < marco_candidato){

				if (tablaPaginasInvertida[marco].pid < -1 && !estaElMarcoReservado(marco, cantRealPaginasSolicitadas, marcosLibres)) {
					marcosLibres[cantPosicionesEncontradas][0] = marco_candidato;
					marcosLibres[cantPosicionesEncontradas][1] = marco;
					cantPosicionesEncontradas++;
					break;
				}
				//Antes de incrementar el marco nos fijamos que no estemos en el final de la memoria
				//y tengamos que empezar desde el principio
				if (marco < config.marcos - 1) {
					marco++;
				} else {
					marco = cantMarcosOcupaTablaPaginas;
				}
			}

			//Si salió porque dio la vuelta y volvió al marco_candidato -> No hay más lugar en memoria
			if ( marco == marco_candidato) {
				return -11;
			}
		}
	}

	/*
	 * Los marcos libres que encontramos previamente y guardamos en la matriz marcosLibres
	 * los usamos para asignar al pid en tablaPaginasInvertida. Guardamos en su respectivo
	 * Overflow a los que hayan colisionado
	 */
	char* buffer = malloc(config.marco_size);
	memset(buffer, '\0', config.marco_size);
	for (i = 0; i < cantRealPaginasSolicitadas; i++) {

		//Si la segunda columna es -1 significa que se el marco es el devuelto por la función hash
		//Si no, el marco utilizado es el encontrado en el recorrido hecho luego de la colisión
		if ( marcosLibres[i][1] == -1) {
			tablaPaginasInvertida[ marcosLibres[i][0] ].pid = pid;
			tablaPaginasInvertida[ marcosLibres[i][0] ].nroPagina = i;
			memcpy(memoria + marcosLibres[i][0] * config.marco_size, buffer, config.marco_size);
		} else {
			tablaPaginasInvertida[ marcosLibres[i][1] ].pid = pid;
			tablaPaginasInvertida[ marcosLibres[i][1] ].nroPagina = i;
			agregarSiguienteEnOverflow(marcosLibres[i][0], marcosLibres[i][1]);
			memcpy(memoria + marcosLibres[i][1] * config.marco_size, buffer, config.marco_size);
		}
	}
	free(buffer);

	//TODO SEMAFORO HASTA ACÁ
	printf("Paginas asignadas con éxito\n");

	return EXIT_SUCCESS;

}

bool estaElMarcoReservado(int marcoBuscado, int cantPaginasSolicitadas, int marcosSolicitados[][2]) {
	int i;
	for (i=0; i < cantPaginasSolicitadas; i++){
		if (marcosSolicitados[i][0] == marcoBuscado || marcosSolicitados[i][1] == marcoBuscado){
			return true;
		}
	}
	return false;
}

void escucharConsolaMemoria(tablaPagina_t* tablaPaginasInvertida) {
	printf("Escuchando nuevas solicitudes de consola en nuevo hilo\n");
	while (1) {
		puts("Ingrese una acción a realizar\n");
		puts("1: Configurar retardo memoria");
		puts("2: Realizar dump de Memoria cache");
		puts("3: Realizar dump de Estructuras de la Memoria");
		puts("4: Realizar dump del contenido de la Memoria completa");
		puts("5: Realizar dump del contenido de la Memoria para un proceso en particular");
		char accion[3];
		if (fgets(accion, sizeof(accion), stdin) == NULL) {
			printf("ERROR AL LEER CONSOLA !\n");
			return;
		}
		int codAccion = accion[0] - '0';
		switch (codAccion) {
		case retardo:
			puts("Ingrese el retardo deseado en milisegundos (Se toman 8 dígitos máximo)");
			char input[10];
			if (fgets(input, sizeof(input), stdin) == NULL) {
				printf("ERROR AL LEER CONSOLA !\n");
				break;
			}
			char *eptr;
			int result = strtol(input, &eptr, 10);
			if (result == 0) {
				printf("Error con el valor ingresado\n");
				break;
			}
			config.retardo_memoria = result;
			printf("Retardo configurado en %d milisegundos\n", config.retardo_memoria);
			break;
		case dumpCache:
			printf("Codificar dumpCache!\n");
			break;
		case dumpEstructurasDeMemoria:
			realizarDumpEstructurasDeMemoria(tablaPaginasInvertida);
			break;
		case dumpMemoriaCompleta:
			realizarDumpContenidoMemoriaCompleta(tablaPaginasInvertida);
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
		default:
			printf("No se reconece la acción %d!\n", codAccion);
		}
	}
}

void atenderHilo(paramHiloDedicado* parametros) {
	int cantBytesRecibidos;
	for (;;) {
		// Gestionar datos de un cliente. Recibimos el código de acción que quiere realizar.
		int codAccion;
		if ((cantBytesRecibidos = recv(parametros->socketClie, &codAccion,
				sizeof(codAccion), 0)) <= 0) {
			// error o conexión cerrada por el cliente
			if (cantBytesRecibidos == 0) {
				// conexión cerrada
				printf("Server: socket %d termino la conexion\n", parametros->socketClie);
				close(parametros->socketClie);
				break;
			} else {
				perror("Se ha producido un error en el Recv");
				break;
			}
		} else {
			printf("He recibido %d bytes con la acción: %d\n",
					cantBytesRecibidos, codAccion);
			pedidoSolicitudPaginas_t pedidoPaginas;
			pedidoBytesMemoria_t pedidoBytes;
			pedidoAlmacenarBytesMemoria_t pedidoAlmacenarBytes;
			//					char* bytesAEscribir;
			char* bytesSolicitados;
			int resultAccion;
			int pidAFinalizar;
			int pagALiberar;

			switch (codAccion) {

			case inicializarProgramaAccion:
				recv(parametros->socketClie, &pedidoPaginas,
						sizeof(pedidoPaginas), 0);
				printf("Recibida solicitud de %d páginas para el pid %d\n",
						pedidoPaginas.cantidadPaginas, pedidoPaginas.pid);
				printf("Se procede a inicializar programa\n");
				resultAccion = inicializarPrograma(pedidoPaginas.pid,
						pedidoPaginas.cantidadPaginas,
						parametros->tablaPaginasInvertida);
				printf(
						"Inicializar programa en Memoria terminó con resultado de acción: %d\n",
						resultAccion);
				send(parametros->socketClie, &resultAccion,
						sizeof(resultAccion), 0);
				break;

			case solicitarPaginasAccion:
				recv(parametros->socketClie, &pedidoPaginas,
						sizeof(pedidoPaginas), 0);
				printf("Recibida solicitud de %d páginas para el pid %d\n",
						pedidoPaginas.cantidadPaginas, pedidoPaginas.pid);
				printf("Se procede a solicitar páginas del programa\n");
				resultAccion = solicitarAsignacionPaginas(pedidoPaginas.pid,
						pedidoPaginas.cantidadPaginas,
						parametros->tablaPaginasInvertida);
				printf(
						"Solicitar páginas adicionales terminó con resultado de acción: %d\n",
						resultAccion);
				send(parametros->socketClie, &resultAccion,
						sizeof(resultAccion), 0);
				break;

			case almacenarBytesAccion:
				recv(parametros->socketClie,
						&pedidoAlmacenarBytes.pedidoBytes.pid,
						sizeof(pedidoAlmacenarBytes.pedidoBytes.pid), 0);
				recv(parametros->socketClie,
						&pedidoAlmacenarBytes.pedidoBytes.nroPagina,
						sizeof(pedidoAlmacenarBytes.pedidoBytes.nroPagina), 0);
				recv(parametros->socketClie,
						&pedidoAlmacenarBytes.pedidoBytes.offset,
						sizeof(pedidoAlmacenarBytes.pedidoBytes.offset), 0);
				recv(parametros->socketClie,
						&pedidoAlmacenarBytes.pedidoBytes.tamanio,
						sizeof(pedidoAlmacenarBytes.pedidoBytes.tamanio), 0);

				pedidoAlmacenarBytes.buffer = malloc(
						pedidoAlmacenarBytes.pedidoBytes.tamanio);

				recv(parametros->socketClie, pedidoAlmacenarBytes.buffer,
						pedidoAlmacenarBytes.pedidoBytes.tamanio, 0);
				//						pedidoAlmacenarBytes.buffer = bytesAEscribir;
				printf(
						"Recibida solicitud de almacenar %d bytes para el pid %d en su página %d\n con un offset de %d\n",
						pedidoAlmacenarBytes.pedidoBytes.tamanio,
						pedidoAlmacenarBytes.pedidoBytes.pid,
						pedidoAlmacenarBytes.pedidoBytes.nroPagina,
						pedidoAlmacenarBytes.pedidoBytes.offset);
				printf("Se procede a almacenar bytes: %s\n",
						pedidoAlmacenarBytes.buffer);
				resultAccion = almacenarBytes(
						pedidoAlmacenarBytes.pedidoBytes.pid,
						pedidoAlmacenarBytes.pedidoBytes.nroPagina,
						pedidoAlmacenarBytes.pedidoBytes.offset,
						pedidoAlmacenarBytes.pedidoBytes.tamanio,
						pedidoAlmacenarBytes.buffer,
						parametros->tablaPaginasInvertida);
				printf(
						"Solicitud de almacenar bytes terminó con resultado de acción: %d\n",
						resultAccion);
				send(parametros->socketClie, &resultAccion,
						sizeof(resultAccion), 0);
				//						free(bytesAEscribir);
				free(pedidoAlmacenarBytes.buffer);
				break;

			case solicitarBytesAccion:
				recv(parametros->socketClie, &pedidoBytes, sizeof(pedidoBytes),0);
				printf(
						"Recibida solicitud de %d bytes para el pid %d en su página %d\n con un offset de %d\n",
						pedidoBytes.tamanio, pedidoBytes.pid,
						pedidoBytes.nroPagina, pedidoBytes.offset);
				printf("Se procede a solicitar bytes\n");
				bytesSolicitados = solicitarBytes(pedidoBytes.pid,
						pedidoBytes.nroPagina, pedidoBytes.offset,
						pedidoBytes.tamanio, parametros->tablaPaginasInvertida);
				memcpy(&resultAccion, bytesSolicitados, sizeof(resultAccion));
				printf(
						"Solicitud de solicitar bytes terminó con resultado de acción: %d\n",
						resultAccion);
				printf("Solicitar bytes devolvió los Bytes solicitados: %s\n",
						bytesSolicitados + sizeof(resultAccion));
				send(parametros->socketClie, bytesSolicitados,
						pedidoBytes.tamanio + sizeof(resultAccion), 0);
				free(bytesSolicitados);
				break;

			case finalizarProgramaAccion:
				recv(parametros->socketClie, &pidAFinalizar, sizeof(pidAFinalizar), 0);
				printf(
						"Recibida solicitud para finalizar programa con pid = %d\n",
						pidAFinalizar);
				printf("Se procede a finalizar el programa\n");
				resultAccion = finalizarPrograma(pidAFinalizar, parametros->tablaPaginasInvertida);
				printf(
						"Solicitud de finalizar programa terminó con resultado de acción: %d\n",
						resultAccion);
				send(parametros->socketClie, &resultAccion, sizeof(resultAccion), 0);
				break;

			case obtenerTamanioPaginas:
				send(parametros->socketClie, &config.marco_size,
						sizeof(int32_t), 0);
				break;

			case accionEnviarStackSize:
				recv(parametros->socketClie, &stack_size, sizeof(int32_t), 0);
				printf("Recibido tamanio dle stack = %d\n", stack_size);
				break;

			case liberarPaginaProcesoAccion:
				recv(parametros->socketClie, &pidAFinalizar, sizeof(pidAFinalizar),0);
				recv(parametros->socketClie, &pagALiberar, sizeof(pagALiberar),0);
				printf("Recibida solicitud para liberar página %d del pid %d\n", pagALiberar, pidAFinalizar);
				resultAccion = liberarPaginaPid(pidAFinalizar, pagALiberar, parametros->tablaPaginasInvertida);
				send(parametros->socketClie, &resultAccion, sizeof(resultAccion), 0);
				break;

			default:
				printf("No reconozco el código de acción\n");
				resultAccion = -13;
				send(parametros->socketClie, &resultAccion,
						sizeof(resultAccion), 0);
			}
			printf("Fin atención acción\n");
		}
	}
}

int main(void) {

	//Setea config_t config
	cargarConfigFile();

	/***********************************************************/

	//Inicializacion tabla de páginas invertida
	tablaPagina_t tablaPaginasInvertida[config.marcos];
	tamanioTablaPagina = config.marcos * sizeof(tablaPagina_t);


	if (tamanioTablaPagina % config.marco_size == 0) {
		cantMarcosOcupaTablaPaginas = (tamanioTablaPagina / config.marco_size);
	} else {
		cantMarcosOcupaTablaPaginas = (tamanioTablaPagina / config.marco_size)
				+ 1;
	}
	printf("Cantidad de marcos que ocupa la tabla de páginas invertida: %d\n",
			cantMarcosOcupaTablaPaginas);

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

	/***********************************************************/

	//Inicialización Overflow Tabla Páginas
	inicializarOverflow(config.marcos);

	/***********************************************************/

	//Inicialización memoria
	tamanioMemoria = config.marco_size * config.marcos;
	printf("Inicializando la memoria de %d bytes\n", tamanioMemoria);
	memoria = malloc(tamanioMemoria);
	memset(memoria, '\0', tamanioMemoria);
	memcpy(memoria, tablaPaginasInvertida, tamanioTablaPagina);
	//Fin inicialización memoria
	printf("Memoria inicializada\n");

	/***********************************************************/

	struct sockaddr_in direccionServidor; // Información sobre mi dirección
	struct sockaddr_in direccionCliente; // Información sobre la dirección del cliente
	socklen_t addrlen; // El tamaño de la direccion del cliente
	int sockServ; // Socket de nueva conexion aceptada
	int sockClie; // Socket a la escucha

	//Crear socket. Dejar reutilizable. Crear direccion del servidor. Bind. Listen.
	sockServ = crearSocket();
	reusarSocket(sockServ, 1);
	direccionServidor = crearDireccionServidor(config.puerto);
	bind_w(sockServ, &direccionServidor);
	listen_w(sockServ);
	printf("Escuchando nuevas solicitudes tcp en el puerto %d...\n",
			config.puerto);
	pthread_t unHilo;
	pthread_create(&unHilo, NULL, (void*) escucharConsolaMemoria,
			(void*) tablaPaginasInvertida);

	// gestionar nuevas conexiones
	addrlen = sizeof(direccionCliente);
	for (;;) {
		if ((sockClie = accept(sockServ, (struct sockaddr*) &direccionCliente,
				&addrlen)) == -1) {
			perror("Error en el accept");
		} else {
			pthread_t hiloDedicado;
			paramHiloDedicado* parametros = malloc(sizeof(paramHiloDedicado));
			parametros->socketClie = sockClie;
			parametros->tablaPaginasInvertida = tablaPaginasInvertida;
			pthread_create(&hiloDedicado, NULL, (void*) atenderHilo,
					(void*) parametros);

			printf("Server: nueva conexion de %s en socket %d\n", inet_ntoa(direccionCliente.sin_addr), sockClie);

		}
	}
}

