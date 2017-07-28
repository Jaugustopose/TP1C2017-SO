#include "memo.h"
#include <time.h>

void cargarConfigFile(char *path) {

	t_config* configMemo = config_create(path);
	if (config_has_property(configMemo, "PUERTO_KERNEL")) {
		config.puerto_kernel = config_get_int_value(configMemo, "PUERTO_KERNEL");
		log_info(memoConsoleLogger, "config.PUERTO_KERNEL: %d", config.puerto_kernel);
	}
	if (config_has_property(configMemo, "IP_KERNEL")) {
		config.ip_kernel = config_get_string_value(configMemo, "IP_KERNEL");
		log_info(memoConsoleLogger, "config.IP_KERNEL: %s", config.ip_kernel);
	}
	if (config_has_property(configMemo, "PUERTO")) {
		config.puerto = config_get_int_value(configMemo, "PUERTO");
		log_info(memoConsoleLogger, "config.PUERTO: %d", config.puerto);
	}
	if (config_has_property(configMemo, "MARCOS")) {
		config.marcos = config_get_int_value(configMemo, "MARCOS");
		log_info(memoConsoleLogger, "config.MARCOS: %d", config.marcos);
	}
	if (config_has_property(configMemo, "MARCO_SIZE")) {
		config.marco_size = config_get_int_value(configMemo, "MARCO_SIZE");
		log_info(memoConsoleLogger, "config.MARCO_SIZE: %d", config.marco_size);
	}
	if(config_has_property(configMemo,"ENTRADAS_CACHE")){
		config.entradas_cache = config_get_int_value(configMemo,"ENTRADAS_CACHE");
		log_info(memoConsoleLogger, "config.ENTRADAS_CACHE: %d", config.entradas_cache);
	}
	if(config_has_property(configMemo, "CACHE_X_PROC")){
		config.cache_x_proc = config_get_int_value(configMemo, "CACHE_X_PROC");
		log_info(memoConsoleLogger, "config.CACHE_X_PROC: %d", config.cache_x_proc);

	}
	if (config_has_property(configMemo, "RETARDO_MEMORIA")) {
		config.retardo_memoria = config_get_int_value(configMemo, "RETARDO_MEMORIA");
		log_info(memoConsoleLogger, "config.RETARDO_MEMORIA: %d", config.retardo_memoria);
	}
}

/* Inicialización vector overflow. Cada posición tiene una
a enlazada que guarda números de frames.
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
	 * recalculo la posición recomenzando desde el principio obviando de nuevo
	 * la tabla de páginas invertida
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
	//pthread_mutex_lock(&lockColisiones);
	//pthread_mutex_lock(&lockTablaPaginas);
	//TODO: REVISAR PORQUE LLEGA INDICE -10
	for (i = 0; i < list_size(overflow[indice]); i++) {
		if (esMarcoCorrecto((int32_t)list_get(overflow[indice], i), pid, pagina, tablaPaginasInvertida)) {
			frame = (int32_t)list_get(overflow[indice], i);
		}
	}
	//pthread_mutex_unlock(&lockTablaPaginas);
	//pthread_mutex_unlock(&lockColisiones);
	return frame;
}

/* Si en la posición candidata de la tabla de páginas invertida se encuentran el pid y páginas recibidos
 * por parámetro, se trata del marco correcto */
int esMarcoCorrecto(int pos_candidata, int pid, int pagina, tablaPagina_t* tablaPaginasInvertida) {
	int esCorrecto = tablaPaginasInvertida[pos_candidata].pid == pid && tablaPaginasInvertida[pos_candidata].nroPagina == pagina;


	return esCorrecto;

}

/* Agrega una entrada a la lista enlazada correspondiente a una posición del vector de overflow */
void agregarSiguienteEnOverflow(int pos_inicial, int nro_frame) {
	//pthread_mutex_lock(&lockColisiones);
	list_add(overflow[pos_inicial], nro_frame);
	//pthread_mutex_unlock(&lockColisiones);
}

/* Elimina un frame de la lista enlazada correspondiente a una determinada posición del vector de overflow  */
void borrarDeOverflow(int posicion, int frame) {
	int i = 0;
	int index_frame;
	//pthread_mutex_lock(&lockColisiones);
	for (i = 0; i < list_size(overflow[posicion]); i++) {
		if (frame == (int) list_get(overflow[posicion], i)) {
			index_frame = i;
			i = list_size(overflow[posicion]);
		}
	}
	list_remove(overflow[posicion], index_frame);
	//pthread_mutex_unlock(&lockColisiones);
}

void realizarDumpMemoriaCache(){
	time_t tm = time(NULL);
	char fechaFormateada[20];
	strftime(fechaFormateada,20,"%Y%m%d_%H%M%S", localtime(&tm));
	char* path = string_from_format("output/dumpMemoriaCache_%s.txt",fechaFormateada);
	FILE* dumpFile = txt_open_for_append(path);
	puts("CONTENIDO MEMORIA CACHÉ:\n\n");
	fprintf(dumpFile,"CONTENIDO MEMORIA CACHÉ:\n\n");
	printf("Se procede a imprimir el contenido de la memoria caché:\n");
		fprintf(dumpFile, "Se procede a imprimir el contenido de la memoria caché:\n");
		printf("%s", cache);
		fprintf(dumpFile, cache);
		fclose(dumpFile);
	}



void realizarDumpEstructurasDeMemoria(tablaPagina_t* tablaPaginasInvertida) {
	time_t tm = time(NULL);
	char fechaFormateada[20];
	strftime(fechaFormateada, 20, "%Y%m%d_%H%M%S", localtime(&tm));
	int i;
	char* path = string_from_format("%s/dumpEstructuras_%s.txt", directorioOutputMemoria, fechaFormateada);
	FILE* dumpFile = txt_open_for_append(path);
	t_list* listaProcesosActivos = list_create();
	//pthread_mutex_lock(&lockTablaPaginas);
	puts("TABLA DE PÁGINAS INVERTIDA");
	fprintf(dumpFile, "TABLA DE PÁGINAS INVERTIDA\n");
	printf("||%*s||%*s||%*s||\n", 9, "Marco  ", 9, "PID   ", 13, "Nro Página");
	fprintf(dumpFile, "||%*s||%*s||%*s||\n", 9, "Marco  ", 9, "PID   ", 13, "Nro Página");
	fprintf(dumpFile, "||==================================||\n");
	for (i = 0; i < config.marcos; i++) {
		printf("||%*d||%*d||%*d||\n", 9, i, 9, tablaPaginasInvertida[i].pid, 12, tablaPaginasInvertida[i].nroPagina);
		fprintf(dumpFile, "||%*d||%*d||%*d||\n", 9, i, 9, tablaPaginasInvertida[i].pid, 12, tablaPaginasInvertida[i].nroPagina);
		//Función privada dentro de este scope (para el closure del find)
		int _soy_pid_buscado(void *p) {
			return p == tablaPaginasInvertida[i].pid;
		}
		//Si no lo encontramos en la lista de activos lo agregamos
		if (list_find(listaProcesosActivos, _soy_pid_buscado) == NULL) {
			list_add(listaProcesosActivos, tablaPaginasInvertida[i].pid);
		}

	}
	//pthread_mutex_unlock(&lockTablaPaginas);
	printf("======================================\n\n");
	fprintf(dumpFile, "======================================\n\n");
	puts("LISTADO DE PROCESOS ACTIVOS");
	fprintf(dumpFile, "LISTADO DE PROCESOS ACTIVOS\n");
	for (i = 0; i < list_size(listaProcesosActivos); i++) {
		int pid = (int) list_get(listaProcesosActivos, i);
		//No imprimimos los nros de pid correspondientes a estructuras administrativas (-1) ni libres (-10)
		if (pid >= 0) {
			printf("PID: %d\n", pid);
			fprintf(dumpFile, "PID: %d\n", pid);
		}
	}
	puts("");
	fprintf(dumpFile, "\n");
	list_destroy(listaProcesosActivos);
	fclose(dumpFile);
}

void realizarDumpContenidoMemoriaCompleta(tablaPagina_t* tablaPaginasInvertida) {

	time_t tm = time(NULL);
	char fechaFormateada[20];
	strftime(fechaFormateada, 20, "%Y%m%d_%H%M%S", localtime(&tm));
	char* path = string_from_format("%s/dumpContenidoMemoria_%s.txt", directorioOutputMemoria, fechaFormateada);
	FILE* dumpFile = txt_open_for_append(path);

	int i;
	char* bufferPagina = malloc(config.marco_size);
	printf("Se procede a imprimir el contenido de cada marco:\n");
	fprintf(dumpFile, "Se procede a imprimir el contenido de cada marco:\n");
	//pthread_mutex_lock(&lockTablaPaginas);
	//pthread_mutex_lock(&lockMemoria);
	for (i = 0; i < config.marcos; i++) {
		memcpy(bufferPagina, memoria + i * config.marco_size, config.marco_size);
		printf("Marco: %d, pid: %d, pag: %d, contenido: %s\n", i,
				tablaPaginasInvertida[i].pid,
				tablaPaginasInvertida[i].nroPagina, bufferPagina);
		fprintf(dumpFile, "Marco: %d, pid: %d, pag: %d, contenido: %s\n",
				i,
				tablaPaginasInvertida[i].pid,
				tablaPaginasInvertida[i].nroPagina, bufferPagina);
	}
	//pthread_mutex_unlock(&lockMemoria);
	//pthread_mutex_unlock(&lockTablaPaginas);
	fclose(dumpFile);
	free(bufferPagina);
}

void realizarDumpContenidoProceso(tablaPagina_t* tablaPaginasInvertida) {

	puts("Ingrese el número de PID");
	char pidInput[100];
	if (fgets(pidInput, sizeof(pidInput), stdin) == NULL) {
		log_error(memoConsoleLogger, "ERROR AL LEER CONSOLA! - pidInput: %s", pidInput);
		return;
	}
	char* eptr;
	int pid = strtol(pidInput, &eptr, 10);
	if (pid == 0) {
		log_error(memoConsoleLogger, "Error con el valor ingresado: %s", pidInput);
		return;
	}
	time_t tm = time(NULL);
	char fechaFormateada[20];
	strftime(fechaFormateada, 20, "%Y%m%d_%H%M%S", localtime(&tm));
	char* path = string_from_format("%s/dumpContenidoPID%d_%s.txt", directorioOutputMemoria, pid, fechaFormateada);
	FILE* dumpFile = txt_open_for_append(path);

	char* bufferPagina = malloc(config.marco_size);
	printf("Se procede a imprimir el contenido de cada marco del pid %d:\n", pid);
	fprintf(dumpFile, "Se procede a imprimir el contenido de cada marco del pid %d:\n", pid);
	int marco;
	int i = 0;
	//pthread_mutex_lock(&lockTablaPaginas);
	while (true) {
		marco = buscarMarco(pid, i, tablaPaginasInvertida);
		if (marco != -10) {
			//pthread_mutex_lock(&lockMemoria);
			memcpy(bufferPagina, memoria + marco * config.marco_size, config.marco_size);
			printf("pag: %d, marco: %d, contenido: %s\n", tablaPaginasInvertida[marco].nroPagina, marco, bufferPagina);
			fprintf(dumpFile, "pag: %d, marco: %d, contenido: %s\n", tablaPaginasInvertida[marco].nroPagina, marco, bufferPagina);
			//pthread_mutex_unlock(&lockMemoria);
		} else {
			break;
		}
		i++;
	}
	//pthread_mutex_unlock(&lockTablaPaginas);

	if (i == 0) {
		printf("El proceso de pid %d no se encuentra cargado en memoria\n\n", pid);
		fprintf(dumpFile, "El proceso de pid %d no se encuentra cargado en memoria\n\n", pid);
	}
	fclose(dumpFile);
	free(bufferPagina);
}

int finalizarPrograma(int pid, tablaPagina_t* tablaPaginasInvertida) {
	int i;
	//-14 = pid no encontrado
	int retorno = -14;
	int nroPagina = 0;
	bool finaliza = false;
	//pthread_mutex_lock(&lockTablaPaginas);
	while(!finaliza){
		int marco_candidato = buscarMarco(pid, nroPagina, tablaPaginasInvertida);
		if (tablaPaginasInvertida[marco_candidato].pid == pid && tablaPaginasInvertida[marco_candidato].pid != -1) {
			tablaPaginasInvertida[marco_candidato].pid = -10;
			tablaPaginasInvertida[marco_candidato].nroPagina = -1;
			retorno = EXIT_SUCCESS;
		} else {
			int marco_real = buscarEnOverflow(marco_candidato, pid, nroPagina, tablaPaginasInvertida);
			if (marco_real > -10 && tablaPaginasInvertida[marco_real].pid == pid && tablaPaginasInvertida[marco_real].pid != -1) {
				tablaPaginasInvertida[marco_real].pid = -10;
				tablaPaginasInvertida[marco_real].nroPagina = -1;
				retorno = EXIT_SUCCESS;
				borrarDeOverflow(marco_candidato, marco_real);
			} else {
				finaliza = true;
			}
		}
		nroPagina++;
	}
	//pthread_mutex_unlock(&lockTablaPaginas);
	if (retorno == -14) {
		log_error(memoLogger, "finalizarPrograma - No se encontró el pid %d. Error code -14", pid);
	}
	return retorno;
}

int liberarPaginaPid(int pid, int nroPagina, tablaPagina_t* tablaPaginasInvertida) {

	int resultAccion;
	//pthread_mutex_lock(&lockTablaPaginas);
	int marco = buscarMarco(pid, nroPagina, tablaPaginasInvertida);
	//pthread_mutex_unlock(&lockTablaPaginas);
	printf("Marco encontrado solicitarBytes: %d\n", marco);
	if (marco == -10) {
		printf("El nro de página %d para el pid %d no existe\n", pid, nroPagina);
		resultAccion = marco;
	} else {
		//pthread_mutex_lock(&lockTablaPaginas);
		//Liberamos Marco
		tablaPaginasInvertida[marco].pid = -10;
		tablaPaginasInvertida[marco].nroPagina = -1;
		//pthread_mutex_unlock(&lockTablaPaginas);
		resultAccion = EXIT_SUCCESS;
	}

	return resultAccion;
}

int solicitarAsignacionPaginas(int pid, int cantPaginas, tablaPagina_t* tablaPaginasInvertida) {
	log_info(memoLogger, "solicitarAsignacionPaginas - Comienza asignación de páginas para pid %d: %d páginas" , pid, cantPaginas);
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

	//		Esta sección está hecha para averiguar los nros de página que debemos
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
	log_info(memoLogger, "solicitarAsignacionPaginas - Último nro de página encontrado para pid %d es: %d", pid, nroPagUltimo);
	//Fin búsqueda nro de página del pid
	nroPag = nroPagUltimo;

	//Recorro la memoria hasta que se termine o la cantidad de marcos libres encontrados satisfaga el pedido
	//pthread_mutex_lock(&lockTablaPaginas);
	for (i = 0; cantidadPosicionesEncontradas < cantPaginas; ++i) {
		nroPag++;
		int marco_candidato = calcularPosicion(pid, nroPag);
		log_info(memoLogger, "solicitarAsignacionPaginas - Para pid %d y página %d se obtuvo el marco candidato: %d", pid, i, marco_candidato);

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
					log_info(memoLogger, "solicitarAsignacionPaginas - El marco candidato %d estaba ocupado. Pid %d y Página %d finalmente asignados al marco %d",
							marco_candidato, pid, nroPag, marco);
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
				log_error(memoLogger, "solicitarAsignacionPaginas - No se encontró espacio en la memoria para alocar el pedido. Retorna código de error -11");
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
	//pthread_mutex_lock(&lockMemoria);
	for (i = 0; i < cantPaginas; i++) {
		//Si la segunda columna es -1 significa que el marco es el devuelto por la función hash
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
	//pthread_mutex_unlock(&lockMemoria);
	//pthread_mutex_unlock(&lockTablaPaginas);

	free(buffer);

	log_info(memoLogger, "solicitarAsignacionPaginas - Paginas asignadas con éxito");

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
char* solicitarBytesMemoria(int pid, int nroPagina, int offset, int tamanio, tablaPagina_t* tablaPaginasInvertida) {
	int codResult;
	char* respuesta = malloc(tamanio + sizeof(codResult));
	char* bytesSolicitados = malloc(tamanio);
	memset(bytesSolicitados, '\0', tamanio);
	memset(respuesta, '\0', tamanio + sizeof(codResult));
	//pthread_mutex_lock(&lockTablaPaginas);
	int marco = buscarMarco(pid, nroPagina, tablaPaginasInvertida);
	//pthread_mutex_unlock(&lockTablaPaginas);
	log_info(memoLogger, "solicitarBytes - Marco encontrado: %d", marco);
	if (marco == -10) {
		codResult = marco;
		log_error(memoLogger, "solicitarBytes - El nro de página %d para el pid %d no existe. Erro Code -10", pid, nroPagina);
	} else if ((offset + tamanio) > config.marco_size) {
		codResult = -12;
		log_error(memoLogger, "solicitarBytes - El pedido de lectura excede el tamaño de la página");
	} else {
		//pthread_mutex_lock(&lockMemoria);
		log_info(memoLogger, "solicitarBytes - Tamaño solicitado: %d", tamanio);
		codResult = EXIT_SUCCESS;
		usleep(config.retardo_memoria * 1000);
		memcpy(bytesSolicitados, memoria + marco * config.marco_size + offset, tamanio);
		//pthread_mutex_unlock(&lockMemoria);
	}
	memcpy(respuesta, &codResult, sizeof(codResult));
	memcpy(respuesta + sizeof(codResult), bytesSolicitados, tamanio);
	return respuesta;

}

int buscarMarco(int pid, int nroPagina, tablaPagina_t* tablaPaginasInvertida) {
	//Los locks se hacen dentro de las funciones esMarcoCorrecto y buscarEnOverflow
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

	log_info(memoLogger, "almacenarBytes - Inicia almacenarBytes");
	if ((offset + tamanio) > config.marco_size) {
		log_error(memoLogger, "almacenarBytes - El pedido excede el tamaño de la página. Error code -12");
		return -12;
	} else {
		//pthread_mutex_lock(&lockTablaPaginas);
		int marcoPagina = buscarMarco(pid, nroPagina, tablaPaginasInvertida);
		//pthread_mutex_unlock(&lockTablaPaginas);
		log_info(memoLogger, "almacenarBytes - Marco candidato para pid %d y nroPagina %d: %d", pid, nroPagina, marcoPagina);
		if (marcoPagina > -1) {
			//pthread_mutex_lock(&lockMemoria);
			char* destino = memoria + marcoPagina * config.marco_size + offset;
			usleep(config.retardo_memoria * 1000);
			memcpy(destino, buffer, tamanio);
			//pthread_mutex_unlock(&lockMemoria);
			log_info(memoLogger, "almacenarBytes - El pedido quedó almacenado");
		} else {
			log_info(memoLogger, "almacenarBytes - Marco no encontrado para pid y nroPagina especificados. Error code -10");
			return -10;
		}
	}

	return EXIT_SUCCESS;
}

int inicializarPrograma(int pid, int cantPaginasSolicitadas, tablaPagina_t* tablaPaginasInvertida) {
	log_info(memoLogger, "inicializarPrograma - Comienza inicialización para pid %d: %d páginas + %d páginas para stack"
			 , pid, cantPaginasSolicitadas, stack_size);
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

	//Recorro la memoria hasta que se termine o la cantidad de marcos libres encontrados satisfaga el pedido
	//pthread_mutex_lock(&lockTablaPaginas);
	for (i = 0; cantPosicionesEncontradas < cantRealPaginasSolicitadas; ++i) {

		int marco_candidato = calcularPosicion(pid, i);
		log_info(memoLogger, "inicializarPrograma - Para pid %d y página %d se obtuvo el marco candidato: %d", pid, i, marco_candidato);

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
					log_info(memoLogger, "inicializarPrograma - El marco candidato %d estaba ocupado. Pid %d y Página %d finalmente asignados al marco %d",
							marco_candidato, pid, i, marco);
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
				log_error(memoLogger, "inicializarPrograma - No se encontró espacio en la memoria para alocar el pedido. Retorna código de error -11");
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
	//pthread_mutex_lock(&lockMemoria);
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
	//pthread_mutex_unlock(&lockMemoria);
	//pthread_mutex_unlock(&lockTablaPaginas);
	free(buffer);

	log_info(memoLogger, "inicializarPrograma - Paginas asignadas con éxito");

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

void obtenerSizeMemoria(tablaPagina_t* tablaPaginasInvertida) {
	int i;
	int cantLibres = 0;
	//pthread_mutex_lock(&lockTablaPaginas);
	for (i = 0; i < config.marcos; i++) {
		if (tablaPaginasInvertida[i].pid == -10) {
			cantLibres++;
		}
	}
	//pthread_mutex_unlock(&lockTablaPaginas);
	log_info(memoConsoleLogger, "obtenerSizeMemoria - Tamaño total de la memoria en frames: %d", config.marcos);
	log_info(memoConsoleLogger, "obtenerSizeMemoria - Cantidad de frames ocupados: %d", config.marcos - cantLibres);
	log_info(memoConsoleLogger, "obtenerSizeMemoria - Cantidad de frames libres: %d", cantLibres);
}

int configurarRetardoMemoria() {
	puts("Ingrese el retardo deseado en milisegundos");
	char input[10];
	if (fgets(input, sizeof(input), stdin) == NULL) {
		log_error(memoConsoleLogger, "ERROR AL LEER CONSOLA! - input: %s", input);
		return 1;
	}
	char* eptr;
	int result = strtol(input, &eptr, 10);
	if (result == 0) {
		log_error("Error con el valor ingresado - input: %s", input);
		result = 1;
	}
	return result;
}

void obtenerSizePid(tablaPagina_t* tablaPaginasInvertida) {
	puts("Ingrese el número de PID");
	char pidInput[1000];
	if (fgets(pidInput, sizeof(pidInput), stdin) == NULL) {
		log_error(memoConsoleLogger, "obtenerSizePid - ERROR AL LEER CONSOLA! - pidInput: %s", pidInput);
		return;
	}
	char* eptr;
	int pid = strtol(pidInput, &eptr, 10);
	if (pid == 0) {
		log_error("obtenerSizePid - Error con el valor ingresado - pidInput: %s", pidInput);
		return;
	}
	int i;
	int cantMarcos = 0;
	log_info(memoLogger, "obtenerSizePid - Se busca pid %d\n", pid);
	//pthread_mutex_lock(&lockTablaPaginas);
	for (i = 0; i < config.marcos; i++) {
		if (tablaPaginasInvertida[i].pid == pid) {
			cantMarcos++;
		}
	}
	//pthread_mutex_unlock(&lockTablaPaginas);
	if (cantMarcos == 0) {
		log_warning(memoConsoleLogger, "El pid %d no se encuentra cargado en memoria!", pid);
	} else {
		log_info(memoConsoleLogger, "El pid %d ocupa %d frames\n", pid, cantMarcos);
	}
}

void escucharConsolaMemoria(tablaPagina_t* tablaPaginasInvertida) {
	log_info(memoLogger, "Escuchando nuevas solicitudes de consola en nuevo hilo");
	int result;
	while (1) {
		puts("Ingrese una acción a realizar\n");
		puts("1: Configurar retardo memoria");
		puts("2: Realizar dump de Memoria cache");
		puts("3: Realizar dump de Estructuras de la Memoria");
		puts("4: Realizar dump del contenido de la Memoria completa");
		puts("5: Realizar dump del contenido de la Memoria para un proceso en particular");
		puts("6: Realizar flush de la Memoria Cache");
		puts("7: Size Memoria (frames, frames ocupados y frames libres)");
		puts("8: Size Proceso");
		char accion[3];
		if (fgets(accion, sizeof(accion), stdin) == NULL) {
			printf("ERROR AL LEER CONSOLA !\n");
			log_error(memoLogger, "ERROR AL LEER LA CONSOLA! - accion: %s", accion);
			return;
		}
		int codAccion = accion[0] - '0';
		switch (codAccion) {
		case retardo:
			result = configurarRetardoMemoria();
			if (result == 1) {
				//Error
				break;
			}
			config.retardo_memoria = result;
			log_info(memoLogger, "Retardo reconfigurado en %d milisegundos", config.retardo_memoria);
			break;
		case dumpCache:
			realizarDumpMemoriaCache();
			break;
		case dumpEstructurasDeMemoria:
			realizarDumpEstructurasDeMemoria(tablaPaginasInvertida);
			break;
		case dumpMemoriaCompleta:
			realizarDumpContenidoMemoriaCompleta(tablaPaginasInvertida);
			break;
		case dumpMemoriaProceso:
			realizarDumpContenidoProceso(tablaPaginasInvertida);
			break;
		case flushCache:
			realizarFlushMemoriaCache();
			break;
		case sizeMemoria:
			obtenerSizeMemoria(tablaPaginasInvertida);
			break;
		case sizePid:
			obtenerSizePid(tablaPaginasInvertida);
			break;
		default:
			printf("No se reconece la acción %d!\n", codAccion);
		}
	}
}

void realizarFlushMemoriaCache(){


	if(!list_is_empty(entradasOcupadasCache))
	{
		list_clean_and_destroy_elements(entradasOcupadasCache,free);
	}

	if(!list_is_empty(entradasLibresCache))
	{
		list_clean_and_destroy_elements(entradasLibresCache,free);
	}

	list_destroy(entradasOcupadasCache);

	list_destroy(entradasLibresCache);

	free(cache);

	inicializarCache();

	//signal(&semaforoCache);

	printf("Cache limpia\n");
}

int32_t entradas_Proceso_En_Cache(int32_t unPid)
{
	int32_t i = 0;
	int32_t cantidadEntradasProceso = 0;
	entradaCache_t* unaEntrada;

	for(i=0;i<list_size(entradasOcupadasCache);i++)
	{
		unaEntrada = list_get(entradasOcupadasCache,i);
		if(unaEntrada->pid == unPid)
			cantidadEntradasProceso ++;
	}

	return cantidadEntradasProceso;
}

int32_t obtener_Indice_Antiguedad_En_Cache(int32_t unPid) {  // Donde 0 es el mas viejo y list_size(entradasOcupadasCache) el mas nuevo.

	int32_t i;
	entradaCache_t* unaEntrada;

	for(i=0;i<list_size(entradasOcupadasCache);i++)
	{
		unaEntrada = list_get(entradasOcupadasCache,i);
		if((unaEntrada->pid) == unPid)
			return i;
	}

	return -1; // Significa que no esta en cache
}



void inicializar_Lista_Entradas_Libres_Cache(t_list* entradasLibresCache){
	int i;
	entradaCache_t* unaEntrada;
	for(i=0;i<config.entradas_cache;i++){
		unaEntrada = malloc(sizeof(entradaCache_t));
		unaEntrada->pid = -1;
		unaEntrada->nroPagina = -1;
		unaEntrada->contenido = cache + i*config.marco_size;
		list_add(entradasLibresCache, unaEntrada);
	}
}

void inicializarCache(){
	tamanioCache = (config.marco_size * config.entradas_cache);
	cache = malloc(tamanioCache);

	entradasLibresCache = list_create();
	entradasOcupadasCache = list_create();

	inicializar_Lista_Entradas_Libres_Cache(entradasLibresCache);

}

bool proceso_Alcanzo_Max_Entradas_Cache(int32_t unPid) {

	return (entradas_Proceso_En_Cache(unPid) == (config.cache_x_proc));
}

int32_t numero_Entrada_Ocupada_Cache(int32_t unPid, int32_t unNroPag){

	uint32_t i;
	entradaCache_t* unaEntrada;

	for (i = 0; i < list_size(entradasOcupadasCache); i++) {
		unaEntrada = list_get(entradasOcupadasCache,i);
		if ((unaEntrada->pid == unPid) && (unaEntrada->nroPagina == unNroPag))
			return i;
	}
	return -1;
}

char* solicitarBytesCache(int32_t unPid, int32_t unNroPag, int32_t unOffset, int32_t unTamanio, int32_t unIndiceCache){

	int32_t codResult = EXIT_SUCCESS;
	entradaCache_t* nuevaEntrada;
	char* bufferAux = malloc(unTamanio);
	char* respuesta = malloc(unTamanio + sizeof(int32_t));
	memset(bufferAux, '\0', unTamanio);
	memset(respuesta, '\0', unTamanio + sizeof(codResult));
	nuevaEntrada = list_get(entradasOcupadasCache,unIndiceCache);
	list_remove(entradasOcupadasCache, unIndiceCache);
	list_add(entradasOcupadasCache, nuevaEntrada);
	//usleep(config.retardo_memoria * 1000);
	memcpy(bufferAux,nuevaEntrada->contenido + unOffset, unTamanio);
	memcpy(respuesta, &codResult,sizeof(codResult));
	memcpy(respuesta + sizeof(codResult), bufferAux,unTamanio);
	return respuesta;
}

void cargar_Nueva_Entrada_En_Cache(int32_t unPid, int32_t unNroPag, tablaPagina_t* tablaPaginasInvertida){

		entradaCache_t* nuevaEntrada;
		int32_t indice;

		if(proceso_Alcanzo_Max_Entradas_Cache(unPid)){ //Proceso alcanza max cantidad de entradas en cache.

			indice = obtener_Indice_Antiguedad_En_Cache(unPid);
			nuevaEntrada = list_get(entradasOcupadasCache,indice);
			list_remove(entradasOcupadasCache,indice);
		}else{ //Proceso no alcanza max cantidad de entradas en cache.
			if(! list_is_empty(entradasLibresCache)){ //Nueva entrada en cache. Todavía tengo espacio libre.
				nuevaEntrada = list_get(entradasLibresCache,0);
				list_remove(entradasLibresCache,0);
			}else{//No tengo espacio en cache. Reemplazo por LRU una pag en cache de otro proceso.
				nuevaEntrada = list_get(entradasOcupadasCache,0);
				list_remove(entradasOcupadasCache,0);
			}
			nuevaEntrada->pid = unPid;
		}
		nuevaEntrada->nroPagina = unNroPag;
		char* buffer = solicitarBytesMemoria(unPid,unNroPag,0,config.marco_size, tablaPaginasInvertida);
		memcpy(nuevaEntrada->contenido,buffer+sizeof(int32_t),config.marco_size);
		list_add(entradasOcupadasCache,nuevaEntrada);

}

void atenderHilo(paramHiloDedicado* parametros) {
	int cantBytesRecibidos;
	int salir=0;
	while (!salir) {
		// Gestionar datos de un cliente. Recibimos el código de acción que quiere realizar.
		int codAccion;
		if ((cantBytesRecibidos = recv(parametros->socketClie, &codAccion, sizeof(codAccion), MSG_WAITALL)) <= 0) {
			// error o conexión cerrada por el cliente
			if (cantBytesRecibidos == 0) {
				// conexión cerrada
				log_error(memoLogger, "atenderHilo - Server: socket %d termino la conexion", parametros->socketClie);
				close(parametros->socketClie);
				salir=true;
				break;
			} else {
				log_error(memoLogger, "Se ha producido un error en el Recv");
				perror("Se ha producido un error en el Recv");
				salir=true;
				break;
			}
		} else {
			log_info(memoLogger, "atenderHilo - He recibido %d bytes con la acción: %d", cantBytesRecibidos, codAccion);
			pedidoSolicitudPaginas_t pedidoPaginas;
			pedidoBytesMemoria_t pedidoBytes;
			pedidoAlmacenarBytesMemoria_t pedidoAlmacenarBytes;
			char* bytesSolicitados;

			int32_t resultAccion;
			int32_t pidAFinalizar;
			int32_t pagALiberar;
			int32_t indiceCache;
			//int32_t indicePidEnCache;
			//entradaCache_t* entradaCache;
			//entradaCache_t* entradaCacheAntigua;
			//entradaCache_t* entradaLibre;

			/*//Función para el closure de find
			bool _soy_pid_buscado_en_cache(entradaCache_t *p) {
			return p->pid == pedidoBytes.pid;
			}*/

			switch (codAccion) {

			case inicializarProgramaAccion:
				recv(parametros->socketClie, &pedidoPaginas, sizeof(pedidoPaginas), MSG_WAITALL);
				log_info(memoLogger, "atenderHilo[inicializarProgramaAccion] - Recibida solicitud de %d páginas para el pid %d", pedidoPaginas.cantidadPaginas, pedidoPaginas.pid);
				log_info(memoLogger, "atenderHilo[inicializarProgramaAccion] - Se procede a inicializar programa");
				resultAccion = inicializarPrograma(pedidoPaginas.pid, pedidoPaginas.cantidadPaginas, parametros->tablaPaginasInvertida);
				log_info(memoLogger, "atenderHilo[inicializarProgramaAccion] - Inicializar programa en Memoria terminó con resultado de acción: %d", resultAccion);
				send(parametros->socketClie, &resultAccion, sizeof(resultAccion), MSG_WAITALL);
				break;

			case solicitarPaginasAccion:
				recv(parametros->socketClie, &pedidoPaginas, sizeof(pedidoPaginas), MSG_WAITALL);
				log_info(memoLogger, "atenderHilo[solicitarPaginasAccion] - Recibida solicitud de %d páginas para el pid %d", pedidoPaginas.cantidadPaginas, pedidoPaginas.pid);
				log_info(memoLogger, "atenderHilo[solicitarPaginasAccion] - Se procede a solicitar páginas del programa");
				resultAccion = solicitarAsignacionPaginas(pedidoPaginas.pid,
						pedidoPaginas.cantidadPaginas,
						parametros->tablaPaginasInvertida);
				log_info(memoLogger, "atenderHilo[solicitarPaginasAccion] - Solicitar páginas adicionales terminó con resultado de acción: %d", resultAccion);
				send(parametros->socketClie, &resultAccion, sizeof(resultAccion), MSG_WAITALL);
				break;

			case almacenarBytesAccion:
				recv(parametros->socketClie, &pedidoAlmacenarBytes.pedidoBytes.pid, sizeof(pedidoAlmacenarBytes.pedidoBytes.pid), MSG_WAITALL);
				recv(parametros->socketClie, &pedidoAlmacenarBytes.pedidoBytes.nroPagina, sizeof(pedidoAlmacenarBytes.pedidoBytes.nroPagina), MSG_WAITALL);
				recv(parametros->socketClie, &pedidoAlmacenarBytes.pedidoBytes.offset, sizeof(pedidoAlmacenarBytes.pedidoBytes.offset), MSG_WAITALL);
				recv(parametros->socketClie, &pedidoAlmacenarBytes.pedidoBytes.tamanio, sizeof(pedidoAlmacenarBytes.pedidoBytes.tamanio), MSG_WAITALL);

				pedidoAlmacenarBytes.buffer = malloc( pedidoAlmacenarBytes.pedidoBytes.tamanio);

				recv(parametros->socketClie, pedidoAlmacenarBytes.buffer, pedidoAlmacenarBytes.pedidoBytes.tamanio, MSG_WAITALL);
				log_info(memoLogger, "atenderHilo[almacenarBytesAccion] - Recibida solicitud de almacenar %d bytes para el pid %d en su página %d con un offset de %d",
						pedidoAlmacenarBytes.pedidoBytes.tamanio,
						pedidoAlmacenarBytes.pedidoBytes.pid,
						pedidoAlmacenarBytes.pedidoBytes.nroPagina,
						pedidoAlmacenarBytes.pedidoBytes.offset);
				log_info(memoLogger, "atenderHilo[almacenarBytesAccion] - Se procede a almacenar bytes: %s", pedidoAlmacenarBytes.buffer);
				resultAccion = almacenarBytes(
						pedidoAlmacenarBytes.pedidoBytes.pid,
						pedidoAlmacenarBytes.pedidoBytes.nroPagina,
						pedidoAlmacenarBytes.pedidoBytes.offset,
						pedidoAlmacenarBytes.pedidoBytes.tamanio,
						pedidoAlmacenarBytes.buffer,
						parametros->tablaPaginasInvertida);

				log_info(memoLogger, "atenderHilo[almacenarBytesAccion] - Solicitud de almacenar bytes terminó con resultado de acción: %d", resultAccion);
				send(parametros->socketClie, &resultAccion, sizeof(resultAccion), MSG_WAITALL);

				//VERIFICO SI EXISTE DATA EN CACHE
				if ((indiceCache = numero_Entrada_Ocupada_Cache(pedidoBytes.pid,pedidoBytes.nroPagina)) != -1){ //Esta en cache. Hay que actualizarla.
					cargar_Nueva_Entrada_En_Cache(pedidoBytes.pid,pedidoBytes.nroPagina,parametros->tablaPaginasInvertida); //Decisión de diseño: Si actualizó páginas en MR también actualizó cache, por lo que esta utilizando la entrada y sería la última entrada utilizada (última en la cola))
				}
				free(pedidoAlmacenarBytes.buffer);
				break;

			case solicitarBytesAccion:

				recv(parametros->socketClie, &pedidoBytes, sizeof(pedidoBytes), MSG_WAITALL);
				log_info(memoLogger, "atenderHilo[solicitarBytesAccion] - Recibida solicitud de %d bytes para el pid %d en su página %d con un offset de %d",
									pedidoBytes.tamanio, pedidoBytes.pid,
									pedidoBytes.nroPagina, pedidoBytes.offset);
				log_info(memoLogger, "atenderHilo[solicitarBytesAccion] - Se procede a solicitar bytes");

				//VERIFICO SI EXISTE DATA EN CACHE
					if ((indiceCache = numero_Entrada_Ocupada_Cache(pedidoBytes.pid,pedidoBytes.nroPagina)) != -1){ //Tenia data en cache
						bytesSolicitados = solicitarBytesCache(pedidoBytes.pid,pedidoBytes.nroPagina,pedidoBytes.offset,pedidoBytes.tamanio, indiceCache);
						send(parametros->socketClie, bytesSolicitados, pedidoBytes.tamanio + sizeof(resultAccion), MSG_WAITALL);
						free(bytesSolicitados);

						break;

					}else{ //Voy a buscar la data a memoria. Cargo una nueva entrada en cache.
						bytesSolicitados = solicitarBytesMemoria(pedidoBytes.pid, pedidoBytes.nroPagina, pedidoBytes.offset, pedidoBytes.tamanio, parametros->tablaPaginasInvertida);
						memcpy(&resultAccion, bytesSolicitados, sizeof(resultAccion));
						send(parametros->socketClie, bytesSolicitados, pedidoBytes.tamanio + sizeof(resultAccion), MSG_WAITALL);
						cargar_Nueva_Entrada_En_Cache(pedidoBytes.pid,pedidoBytes.nroPagina,parametros->tablaPaginasInvertida);
						free(bytesSolicitados);

						break;

					}

				/*printf("Se procede a solicitar bytes\n");
				bytesSolicitados = solicitarBytesMemoria(pedidoBytes.pid,
						pedidoBytes.nroPagina, pedidoBytes.offset,
						pedidoBytes.tamanio, parametros->tablaPaginasInvertida);
				memcpy(&resultAccion, bytesSolicitados, sizeof(resultAccion));
				log_info(memoLogger, "atenderHilo[solicitarBytesAccion] - Solicitud de solicitar bytes terminó con resultado de acción: %d", resultAccion);
				log_info(memoLogger, "atenderHilo[solicitarBytesAccion] - Solicitar bytes devolvió los Bytes solicitados: %s",
										bytesSolicitados + sizeof(resultAccion));
				send(parametros->socketClie, bytesSolicitados,
						pedidoBytes.tamanio + sizeof(resultAccion), 0);
				free(bytesSolicitados);

				break;*/

			case finalizarProgramaAccion:
				recv(parametros->socketClie, &pidAFinalizar, sizeof(pidAFinalizar), MSG_WAITALL);
				log_info(memoLogger, "atenderHilo[finalizarProgramaAccion] - Recibida solicitud para finalizar programa con pid = %d",
						pidAFinalizar);
				log_info(memoLogger, "atenderHilo[finalizarProgramaAccion] - Se procede a finalizar el programa");
				resultAccion = finalizarPrograma(pidAFinalizar, parametros->tablaPaginasInvertida);
				log_info(memoLogger, "atenderHilo[finalizarProgramaAccion] - Solicitud de finalizar programa terminó con resultado de acción: %d",
						resultAccion);
				send(parametros->socketClie, &resultAccion, sizeof(resultAccion), MSG_WAITALL);
				break;

			case obtenerTamanioPaginas:
				log_info(memoLogger, "atenderHilo[obtenerTamanioPaginas] - Se procede a enviar tamaño del marco: %d", config.marco_size);
				send(parametros->socketClie, &config.marco_size, sizeof(int32_t), MSG_WAITALL);
				break;

			case accionEnviarStackSize:
				recv(parametros->socketClie, &stack_size, sizeof(int32_t), MSG_WAITALL);
				log_info(memoLogger, "atenderHilo[accionEnviarStackSize] - Recibido tamanio del stack: %d", stack_size);
				break;

			case liberarPaginaProcesoAccion:
				recv(parametros->socketClie, &pidAFinalizar, sizeof(pidAFinalizar),MSG_WAITALL);
				recv(parametros->socketClie, &pagALiberar, sizeof(pagALiberar),MSG_WAITALL);
				log_info(memoLogger, "atenderHilo[liberarPaginaProcesoAccion] - Recibida solicitud para liberar página %d del pid %d", pagALiberar, pidAFinalizar);
				resultAccion = liberarPaginaPid(pidAFinalizar, pagALiberar, parametros->tablaPaginasInvertida);
				log_info(memoLogger, "atenderHilo[liberarPaginaProcesoAccion] - Solicitud de liberar pagina de proceso terminó con resultado de acción: %d",
						resultAccion);
				send(parametros->socketClie, &resultAccion, sizeof(resultAccion), MSG_WAITALL);
				break;

			default:
				log_warning(memoLogger, "atenderHilo[default] - No se reconoce el código de acción %d. Error code -13", codAccion);
				resultAccion = -13;
				send(parametros->socketClie, &resultAccion, sizeof(resultAccion), MSG_WAITALL);
			}
			log_info(memoLogger, "atenderHilo- Fin atención acción");
		}
	}
}

void inicializarLog() {
	char* filepath = string_new();
	string_append(&filepath, directorioOutputMemoria);
	string_append(&filepath, "/");
	string_append(&filepath, memoriaLogFileName);
	memoLogger = log_create(string_from_format("%s.log", filepath), "Memoria", false, LOG_LEVEL_INFO);
	memoConsoleLogger = log_create(string_from_format("%s.log", filepath), "Memoria", true, LOG_LEVEL_INFO);
}

void inicializarTablaPaginas(tablaPagina_t tablaPaginasInvertida[config.marcos]) {
	if (tamanioTablaPagina % config.marco_size == 0) {
		cantMarcosOcupaTablaPaginas = (tamanioTablaPagina / config.marco_size);
	} else {
		cantMarcosOcupaTablaPaginas = (tamanioTablaPagina / config.marco_size) + 1;
	}
	log_info(memoLogger, "Cantidad de marcos que ocupa la tabla de páginas invertida: %d",
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
	log_info(memoLogger, "Tabla de páginas invertida inicializada");
}

void inicializarMutex() {
	//pthread_mutex_init(&lockMemoria, NULL);
	//pthread_mutex_init(&lockTablaPaginas, NULL);
	//pthread_mutex_init(&lockColisiones, NULL);
}

void finalizar() {
	log_destroy(memoLogger);
	log_destroy(memoConsoleLogger);
	//pthread_mutex_destroy(&lockMemoria);
	//pthread_mutex_destroy(&lockTablaPaginas);
	//pthread_mutex_destroy(&lockColisiones);
	free(memoria);
}

void sig_handler(int signo) {
  if (signo == SIGINT) {
	  printf("SIGINT interceptado. Finalizando... \n");
	  //TODO IMPLEMENTAR LO NECESARIO PARA NO ROMPER A LOS OTROS PROCESOS
	  exit(EXIT_SUCCESS);
  }
}

int main(int argc, char *argv[]) {
//	printf(argc);
	if(argc > 1) {
		if (signal(SIGINT, sig_handler) == SIG_ERR)
		  printf("Error al interceptar SIGINT\n");

		/***********************************************************
		***Si no existe el directorio de output lo crea*************/
		struct stat st = {0};
		if (stat(directorioOutputMemoria, &st) == -1) {
			mkdir(directorioOutputMemoria, 0700);
		}
		/***********************************************************/

		inicializarLog();

		/***********************************************************/
		//Setea config_t config
		cargarConfigFile(argv[1]);

		/***********************************************************/

		//Inicializacion tabla de páginas invertida
		tablaPagina_t tablaPaginasInvertida[config.marcos];
		tamanioTablaPagina = config.marcos * sizeof(tablaPagina_t);


		inicializarTablaPaginas(tablaPaginasInvertida);
		//Fin inicialización tabla de páginas invertida

		/***********************************************************/
		//Inicialización Overflow Tabla Páginas
		inicializarOverflow(config.marcos);

		/***********************************************************/
		//Inicialización memoria
		tamanioMemoria = config.marco_size * config.marcos;
		log_info(memoLogger, "Inicializando la memoria de %d bytes", tamanioMemoria);
		memoria = malloc(tamanioMemoria);
		memset(memoria, '\0', tamanioMemoria);
		memcpy(memoria, tablaPaginasInvertida, tamanioTablaPagina);
		//Fin inicialización memoria
		log_info(memoLogger, "Memoria inicializada");

		/***********************************************************/
		//Inicialización memoria caché

		inicializarCache();

		/***********************************************************/
		inicializarMutex();

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
		log_info(memoLogger, "Escuchando nuevas solicitudes tcp en el puerto %d...", config.puerto);
		pthread_t unHilo;
		pthread_create(&unHilo, NULL, (void*) escucharConsolaMemoria, (void*) tablaPaginasInvertida);

		// gestionar nuevas conexiones
		addrlen = sizeof(direccionCliente);
		for (;;) {
			if ((sockClie = accept(sockServ, (struct sockaddr*) &direccionCliente, &addrlen)) == -1) {
				log_error(memoLogger, "Error en el accept");
				perror("Error en el accept");
			} else {
				pthread_t hiloDedicado;
				paramHiloDedicado* parametros = malloc(sizeof(paramHiloDedicado));
				parametros->socketClie = sockClie;
				parametros->tablaPaginasInvertida = tablaPaginasInvertida;
				pthread_create(&hiloDedicado, NULL, (void*) atenderHilo, (void*) parametros);

				log_info(memoLogger, "Server: nueva conexion de %s en socket %d", inet_ntoa(direccionCliente.sin_addr), sockClie);

			}
		}

		finalizar();
	} else {
		printf("Es necesario que se pase el path por parámetro\n");
	}
}
