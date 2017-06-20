#include "CapaMemoria.h"

bool peticion_valida(int tamanio) {
	return (tamanioPag - 10) <= tamanio;
}

int tamanioDisponibleDeLaPagina(int tamanioDispActual, int espacioPedido) {
	return (tamanioDispActual - 10 - espacioPedido);
}

bool existePid(t_pidHeap* elem) {
	return (elem->pid == pidCondicion);
}

bool paginaDisponible(t_paginaHeap* elem) {
	return (elem->tamDisponible <= solicitudCondicion);
}

bool existeBloque(t_paginaHeap* elem) {
	return list_size(elem->bloques) > 0 ? true : false;
}

void solicitarPaginaHeap() {
	//Envia Solicitud de paginas HEAP
	int offset = 0;
	int codigoAccion = solicitarPaginasAccion;
	char* bufferMemoria = malloc(
			sizeof(codigoAccion) + sizeof(pedidoSolicitudPaginas_t));

	pedidoSolicitudPaginas_t pedido;
	pedido.pid = pidCondicion;
	pedido.cantidadPaginas = 1;

	memcpy(bufferMemoria, &codigoAccion, sizeof(codigoAccion));
	offset += sizeof(codigoAccion);
	memcpy(bufferMemoria + offset, &pedido, sizeof(pedidoSolicitudPaginas_t));
	offset += sizeof(pedidoSolicitudPaginas_t);

	send(memoria, bufferMemoria, offset, 0);

	char* resultadoBuffer = malloc(sizeof(int));
	recv(memoria, resultadoBuffer, sizeof(int), 0);
	int* resultado;
	deserializar_int(&resultado, resultadoBuffer);

	//interpretar resultado

	free(bufferMemoria);
	free(resultadoBuffer);

}

void crearPaginaHeap() {
	t_pidHeap* pidHeap = list_find(tablaPaginasHeap, (void*) existePid);
}

bool bloqueConTamanioDisponible(heapMetadata* metadata, int espacio) {

	return (metadata->isFree && (metadata->size >= (espacio + 5)));
}

bool bloqueDisponible(heapMetadata* bloque) {
	return (bloque->isFree && (bloque->size >= solicitudCondicion));
}

heapMetadata* proximoBloqueDisponible(t_paginaHeap* pagina) {
	return list_find(pagina->bloques, (void*) bloqueDisponible);
}

void alocar_espacio() {
	t_pidHeap* pidHeap = list_find(tablaPaginasHeap, (void*) existePid);
	t_paginaHeap* paginaHeap = list_find(pidHeap->paginas,
			(void*) paginaDisponible);

	int cantElementosLista = list_size(paginaHeap->bloques);
	heapMetadata* ultimoBloque = list_get(paginaHeap->bloques,
			cantElementosLista);

	//Busca en todas las paginas el proximo bloque libre con espacio disponible
	//heapMetadata* bloque = list_find(pidHeap->paginas,(void*)proximoBloqueDisponible);

	if (bloqueConTamanioDisponible(ultimoBloque, solicitudCondicion)) {
		int libreAnterior = ultimoBloque->size;
		int nuevoEspacioLibre = libreAnterior - solicitudCondicion;

		//Actualizo el ultimo bloque
		ultimoBloque->isFree = false;
		ultimoBloque->size = solicitudCondicion;

		//Agrego un nuevo bloque a la ultima pagina
		heapMetadata* nuevoBloque = malloc(sizeof(heapMetadata));
		nuevoBloque->isFree = true;
		nuevoBloque->size = nuevoEspacioLibre;
		list_add(paginaHeap->bloques, nuevoBloque);

	} else {
		solicitarPaginaHeap();
	}

}

void crearPidHeap(int pid) {
	t_pidHeap* elemento = malloc(sizeof(t_pidHeap));
	elemento->pid = pid;
	elemento->paginas = list_create();

	t_paginaHeap* pagina = malloc(sizeof(t_paginaHeap));
	pagina->bloques = list_create();
	pagina->tamDisponible = tamanioPag;

	heapMetadata* bloque = malloc(sizeof(uint32_t) + sizeof(_Bool));
	bloque->size = tamanioPag - 5;
	bloque->isFree = true;

	list_add(pagina->bloques, bloque);
	list_add(elemento->paginas, pagina);
	list_add(tablaPaginasHeap, elemento);
}

void destructorBloque(heapMetadata* bloque) {
	free(bloque);
}

void desfragmentar(t_list* bloques) {
//PODRIA DEFRAGMENTAR ANTES DE CADA GUARDADO EN EL HEAP
	int desde = 0;
	int cantBloquesContiguos = 0;
	int i = 0;
	int size = 0;
	int indice = 0;
	bool hayUnoMas;
	heapMetadata* bloqueDisponible;
	heapMetadata* bloque;

	while (i < list_size(bloques) && hayUnoMas) {
		bloque = list_get(bloques, i);
		if (bloque->isFree) {
			if (desde != 0) {
				desde = i;
				bloqueDisponible = bloque;
			}
			cantBloquesContiguos++;
			size += bloque->size;
			hayUnoMas = true;
		} else {
			hayUnoMas = false;
		}
		i++;
	}

	indice = desde;
	for (j = 0; j < cantBloquesContiguos; j++) {

		list_remove_and_destroy_element(bloques, indice, (void*)destructorBloque);

		indice++;
	}
	heapMetadata* bloqueNuevo = malloc(sizeof(heapMetadata));
	bloqueNuevo->isFree = false;
	bloqueNuevo->size = size;
	list_add_in_index(bloques, desde, bloqueNuevo);

}

void buscarPIDHeap(int pid, int solicitud) {
	pidCondicion = pid;
	solicitudCondicion = solicitud;
	t_pidHeap* elemento = list_find(tablaPaginasHeap, (void*) existePid);

	if (elemento == NULL) {
		solicitarPaginaHeap();
		crearPidHeap(pid);
		alocar_espacio();
	} else {

		t_paginaHeap* pagina = list_find(elemento->paginas,
				(void*) paginaDisponible);

		if (pagina == NULL) {
			solicitarPaginaHeap();
//TODO:crear nueva pagina
		} else {

//heapMetadata* bloqueDisponible = proximoBloqueDisponible(pagina);
			int posicion;
			int j = 0;
			bool encontrado = false;
			heapMetadata* bloqueDisponible;

			while (j < list_size(pagina->bloques) && !encontrado) {
				bloqueDisponible = list_get(pagina->bloques, j);
				if (bloqueDisponible->isFree
						&& (bloqueDisponible->size >= solicitudCondicion)) {
					posicion = list_get(pagina->bloques, j);
					encontrado = true;
				}
				j++;
			}

			if (bloqueDisponible == NULL) {
				solicitarPaginaHeap();
//TODO:crear nueva pagina
			} else {
//Encontro un bloque donde alocar espacio
				int tamanioNoUsado = bloqueDisponible->size
						- solicitudCondicion;

				bloqueDisponible->isFree = false;
				bloqueDisponible->size = solicitudCondicion;

//Si queda espacio libre, crea el bloque de espacio libre
				if (tamanioNoUsado != 0) {
					heapMetadata* bloqueRestante = malloc(sizeof(heapMetadata));
					bloqueRestante->isFree = true;
					bloqueRestante->size = tamanioNoUsado;

					list_add_in_index(pagina->bloques, posicion + 1,
							bloqueDisponible);
				}

//Actualiza el tamaño disponible de la pagina
				pagina->tamDisponible -= tamanioNoUsado;
			}
		}

		alocar_espacio();
	}
}
