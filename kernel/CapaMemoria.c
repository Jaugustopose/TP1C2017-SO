#include "CapaMemoria.h"

//De esos 10: 5 para su propio heapMetadata y otros 5 para el bloque que siempre esta en la lista.
bool solicitudValida(int espacioSolicitado)
{
	return (espacioSolicitado <= (tamanioPag - 10));
}

t_pidHeap* getPID(int pid)
{
  bool porPID(t_pidHeap* entradaTabla){
				return entradaTabla->pid == pid;
   }

  t_pidHeap* entrada = list_find(listaPidHEAP, (void*)porPID);
  return entrada;
}

t_paginaHeap* getPagina(t_pidHeap* pidElement, int nroPag)
{
  bool porPagina(t_paginaHeap* entradaTabla){
				return entradaTabla->nro == nroPag;
   }

  t_paginaHeap* entrada = list_find(pidElement->paginas, (void*) porPagina);

  return entrada;
}

t_bloque* getBloque(t_paginaHeap* pagina, int indice)
{
  bool porBloque(t_bloque* entradaTabla){
				return entradaTabla->indice == indice;
   }

  t_bloque* entrada = list_find(pagina->bloques, (void*)porBloque);

  return entrada;
}
//
//OJO:Paginas que arrancan a numerarse en 0
int getLastNroPag(int pid)
{
	t_pidHeap* elementoPid = getPID(pid);
	int numeroNuevo = 0;

	if(elementoPid != NULL)
	{
		numeroNuevo = list_size(elementoPid->paginas);
	}

	return numeroNuevo;
}

void solicitarPagina(int pid)
{
	void* buffer = malloc(sizeof(int) + sizeof(pedidoSolicitudPaginas_t));

	int codAccion = solicitarPaginasAccion;
	pedidoSolicitudPaginas_t solicitud;
	solicitud.pid = pid;
	solicitud.cantidadPaginas = 1;

	memcpy(buffer, &codAccion, sizeof(codAccion));
	memcpy(buffer + sizeof(codAccion), &solicitud, sizeof(pedidoSolicitudPaginas_t));

	send(memoria, buffer, sizeof(codAccion) + sizeof(pedidoSolicitudPaginas_t), 0);

	char* stackOverflow = malloc(sizeof(int));
	int bytesRecibidos = recv(memoria, stackOverflow, sizeof(int), 0);
	int overflow = char4ToInt(stackOverflow);
	free(stackOverflow);
	free(buffer);

	t_paginaHeap *paginaNueva = malloc(sizeof(t_paginaHeap));
	paginaNueva->pid = pid;
	paginaNueva->nro = getLastNroPag(pid);
	paginaNueva->tamDisponible = tamanioPag - 10;
	paginaNueva->bloques = list_create();

	t_bloque *bloqueInicial = malloc(sizeof(t_bloque));
	//bloqueInicial->metadata = malloc(sizeof(t_heapMetadata));
    bloqueInicial->isFree = true;
    bloqueInicial->size = tamanioPag - 5;
    bloqueInicial->indice = 0;

    list_add(paginaNueva->bloques, bloqueInicial);

    t_pidHeap* pidElemento = getPID(pid);

    if(pidElemento == NULL)
    {
    	//Creo el elemento pid porque es la primera alocacion del proceso
    	t_pidHeap *pidNuevo = malloc(sizeof(t_pidHeap));
    	pidNuevo->pid = pid;
    	pidNuevo->paginas = list_create();

    	list_add(pidNuevo->paginas, paginaNueva);
    	list_add(listaPidHEAP, pidNuevo);
    }else
    {
    	list_add(pidElemento->paginas, paginaNueva);
    }

    t_proceso* proceso = buscarProcesoPorPID(pid);
    proceso->cantidadPaginasHeap++;

}

t_paginaHeap* getPaginaConEspacio(t_pidHeap* pidElement, int pid, int espacio)
{
	bool porPIDYTamanio(t_paginaHeap* entrada){
		return (entrada->pid == pidElement->pid) && (entrada->tamDisponible >= espacio);
	}

  t_paginaHeap* pagina = list_find(pidElement->paginas, (void*) porPIDYTamanio);
  return pagina;
}

t_bloque* getBloqueConEspacio(t_paginaHeap* pagina, int espacio)
{
	bool porBloque(t_bloque* entradaTabla){
		return entradaTabla->isFree && (entradaTabla->size >= espacio);
	}

 t_bloque* bloque = list_find(pagina->bloques, (void*) porBloque);
 return bloque;
}

//OJO: Bloques que arrancan a numerarse en CERO
t_puntero alocar(int pid, int espacio)
{
	t_pidHeap* elementoPID = getPID(pid);
	t_paginaHeap* elementoPagina = getPaginaConEspacio(elementoPID, pid, espacio);
	t_bloque* bloque = getBloqueConEspacio(elementoPagina, espacio);

	t_bloque* bloqueNuevo = malloc(sizeof(t_bloque));
	bloqueNuevo->indice = bloque->indice;
	bloqueNuevo->size = espacio;
	bloqueNuevo->isFree = false;

	t_bloque* bloqueViejo = list_remove(elementoPagina->bloques, bloque->indice);
	bloqueViejo->indice = list_size(elementoPagina->bloques) + 1;
	bloqueViejo->isFree = true;
	bloqueViejo->size = bloqueViejo->size - espacio;

	list_add_in_index(elementoPagina->bloques, bloqueNuevo->indice, bloqueNuevo);
	list_add(elementoPagina->bloques, bloqueViejo);

	elementoPagina->tamDisponible = elementoPagina->tamDisponible - espacio;

	int posicion = 5;
	int tope = bloqueNuevo->indice;

	void calcularPosicion(t_bloque* bloque)
	{
		if(bloque->indice < tope)
		{
			posicion = posicion + bloque->size + 5;
		}
	}

	list_iterate(elementoPagina->bloques,(void*)calcularPosicion);

    t_proceso* proceso = buscarProcesoPorPID(pid);
    proceso->bytesAlocados = proceso->bytesAlocados + espacio + 5;

	//Devuelve una posicion absoluta tomando en cuenta PAGINAS DE STACK, de HEAP y OFFSET de bloques.
	//Es resonsabilidad de las primitivas de CPU sumarle las paginas de codigo.
	t_puntero puntero = ((config.STACK_SIZE + elementoPagina->nro) * tamanioPag) + posicion;

	return puntero;

}

t_puntero alocarMemoria(int espacioSolicitado, int pid)
{
	if(solicitudValida(espacioSolicitado))
	{

		t_pidHeap* elementoPID = getPID(pid);
		if(elementoPID != NULL)
		{

			t_paginaHeap* pagina  = getPaginaConEspacio(elementoPID, pid, espacioSolicitado);
			if(pagina != NULL)
			{

				t_bloque* bloque = getBloqueConEspacio(pagina, espacioSolicitado);
				if(bloque != NULL)
				{
					int pun = alocar(pid, espacioSolicitado);
					return pun;

				}else
				{
					//es posible desfragmentar?
					int32_t tamanioRecuperado = defragmentar(pagina);
					if(tamanioRecuperado >= espacioSolicitado)
					{
						return alocar(pid, espacioSolicitado);
					}else
					{
						//NO ES POSIBLE DESFRAGMENTAR
						solicitarPagina(pid);
						return alocar(pid, espacioSolicitado);
					}
				}

			}else
			{
				solicitarPagina(pid);
				int valor = alocar(pid, espacioSolicitado);
				return valor;
			}

		}else
		{
			solicitarPagina(pid);
			return alocar(pid, espacioSolicitado);
		}
	}
	else
	{
		//RECHAZO: Solicitud invalida
		return -1;
	}
}

void bloquesDestroyer(t_bloque* bloque) {
    free(bloque);
}

void paginasDestroyer(t_paginaHeap* pagina)
{
	free(pagina);
}

void liberarPaginaEstructura(t_paginaHeap* paginaALiberar, t_pidHeap* pidElement)
{
	int nroPagAEliminar = paginaALiberar->nro;

	bool paginaBuscada(t_paginaHeap* pagina){
		return pagina->nro == nroPagAEliminar;
	}

	list_destroy_and_destroy_elements(paginaALiberar->bloques, (void*)bloquesDestroyer);
	list_remove_and_destroy_by_condition(pidElement->paginas, (void*)paginaBuscada, (void*)paginasDestroyer);
}

bool bloquesTodosFree(t_paginaHeap* pagina)
{
	bool estaLibre_bloque(t_bloque* bloque)
	{
		return bloque->isFree == true;
	}

  return list_all_satisfy(pagina->bloques, (void*)estaLibre_bloque);
}

int calcularIndiceBloque(t_paginaHeap* pagina, int offset)
{
	int bytes = 5;
	int indice = 0;
	void calculaIndice(t_bloque* bloque)
	{
		if(bytes < offset)
		{
			bytes = bytes +( 5 + bloque->size);
			indice++;
		}
	}

	list_iterate(pagina->bloques, (void*)calculaIndice);
	return indice;
}

int liberarMemoria(t_puntero puntero, int pid, int cantPaginasCodigo)
{
   int nroPagina = (int)(puntero/tamanioPag);
   nroPagina = nroPagina - config.STACK_SIZE;
   int pidLiberar = pid;
   int offset = puntero - ((nroPagina + config.STACK_SIZE) * tamanioPag); //hacerloPositivo

   t_pidHeap* pidElement = getPID(pid);
   t_paginaHeap* pagina = getPagina(pidElement, nroPagina);

   int indice = calcularIndiceBloque(pagina, offset);

   t_bloque* bloque = getBloque(pagina, indice);
   bloque->isFree = true;
   pagina->tamDisponible = pagina->tamDisponible + bloque->size;

	t_proceso* proceso = buscarProcesoPorPID(pagina->pid);
	proceso->bytesLiberados = proceso->bytesLiberados + bloque->size;

	if(bloquesTodosFree(pagina))
	{
		liberarPagina(pagina, puntero, cantPaginasCodigo);
		liberarPaginaEstructura(pagina, pidElement);
	}

	return 1;
}

//LLega con la cantidad de paginas de codigo?
void liberarPagina(t_paginaHeap* pagina, int puntero, int cantPaginasCodigo)
{
	int pidLiberar = pagina->pid;
	int nroPagina = (int)((puntero/tamanioPag) + cantPaginasCodigo);

	void* buffer = malloc(sizeof(int)*3);

	int codAccion = liberarPaginaProcesoAccion;

	memcpy(buffer, &codAccion, sizeof(codAccion));
	memcpy(buffer + sizeof(codAccion), &pidLiberar, sizeof(pidLiberar));
	memcpy(buffer + sizeof(codAccion) + sizeof(nroPagina), &nroPagina, sizeof(nroPagina));

	send(memoria, buffer, sizeof(int)*3, 0);

	char* stackOverflow = malloc(sizeof(int));
	int bytesRecibidos = recv(memoria, stackOverflow, sizeof(int), 0);
	int overflow = char4ToInt(stackOverflow);
	free(stackOverflow);

	t_proceso* proceso = buscarProcesoPorPID(pagina->pid);
	proceso->cantidadPaginasHeap--;

}

int32_t defragmentar(t_paginaHeap* pagina)
{

	/*DESFRAGMENTAR busca en una lista de bloques la primer secuencia de bloques contigua.
	 * Se guarda el tamaño que puede recuperar y la posicion donde hacerlo.
	 * Se crea un nuevo bloque con ese tamaño en esa posicion y se lo marca como libre.
	 * Se renombran los indices de la lista de bloques para que vuelvan a estar incrementales.
	 * Se actualiza el tamaño de paginas.
	 * Los casos salvabales son: bloques contiguos.
	 * Si hay 3 secuencias contiguas no consecutivas en una misma pagina, se llamara a la funcion defragmentar 3 veces
	 */

	int32_t indiceInicial = 0;
	int32_t indiceFinal = 0;
	int32_t posicionInicial= 0;
	int32_t posicionFinal= 0;
	int32_t bytes = 0;
	bool hayUnaSecuenciaContigua = true;

	void encontrarSecuenciaDeBloques(t_bloque* bloque)
		{
			if(hayUnaSecuenciaContigua)
			{
				if(bloque->isFree && indiceInicial != 0)
				{
					posicionInicial = bytes;
					indiceInicial = bloque->indice;
				}else if(bloque->isFree)
				{
					posicionFinal = bytes + 5 + bloque->size;
					indiceFinal = bloque->indice;
					hayUnaSecuenciaContigua = true;
				}

				bytes = bytes +5+ bloque->size;
		  }
		}
	bool esEliminable(t_bloque* unBloque)
	{
		return ((unBloque->indice >= posicionInicial) &&  (unBloque->indice <= posicionFinal));
	}
	list_iterate(pagina->bloques,(void*)encontrarSecuenciaDeBloques);

	list_remove_and_destroy_by_condition(pagina->bloques,(void*)esEliminable, (void*)bloquesDestroyer);

	t_bloque* bloqueNuevo = malloc(sizeof(t_bloque));
	bloqueNuevo->indice = indiceInicial; //CAMBIAAAAAR
	bloqueNuevo->isFree = true;
	bloqueNuevo->size = bytes - 5;

	list_add_in_index(pagina->bloques, bloqueNuevo->indice, bloqueNuevo);

	int start = 0;


	void actualizaIndice(t_bloque* unBloque)
	{
		unBloque->indice = 0;
		start++;
	}
	list_iterate(pagina->bloques,(void*)actualizaIndice);

	pagina->tamDisponible = pagina->tamDisponible + bytes - 5;

	int32_t tamanioNuevoDisponible = bytes - 5;
	return tamanioNuevoDisponible;

}


