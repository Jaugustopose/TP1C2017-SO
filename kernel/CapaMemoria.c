#include "CapaMemoria.h"

bool solicitudValida(int espacioSolicitado)
{
	return (espacioSolicitado <= (tamanioPag - 10));
}

t_pidHeap* getPID(int pid)
{
  bool porPID(t_pidHeap* entradaTabla, int pid){
				return entradaTabla->pid == pid;
   }

  t_pidHeap* entrada = list_find(listaPidHEAP, (void*)porPID);
  return entrada;
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

}

t_paginaHeap* getPaginaConEspacio(t_pidHeap* pidElement, int pid, int espacio)
{
	bool porPagina(t_paginaHeap* entradaTabla, int pid){
						return entradaTabla->pid == pid && (entradaTabla->tamDisponible >= espacio);
	}

  t_paginaHeap* pagina = list_find(pidElement->paginas, (void*)porPagina);
  return pagina;
}

t_bloque* getBloqueConEspacio(t_paginaHeap* pagina, int espacio)
{
	bool porBloque(t_bloque* entradaTabla, int pid){
		return entradaTabla->metadata->isFree && (entradaTabla->metadata->size >= espacio);
	}

	t_bloque* bloque = list_find(pagina->bloques, (void*)porBloque);
 return bloque;
}

t_puntero alocar(int pid, int espacio)
{
	t_pidHeap* elementoPID = getPID(pid);
	t_paginaHeap* pagina = getPaginaConEspacio(elementoPID, elementoPID->pid, espacio);
	t_bloque* bloque = getBloqueConEspacio(pagina, espacio);

	t_bloque* bloqueNuevo = malloc(sizeof(t_bloque));
	bloqueNuevo->indice = bloque->indice;
	bloqueNuevo->metadata = malloc(sizeof(heapMetadata));
	bloqueNuevo->metadata->size = espacio;
	bloqueNuevo->metadata->isFree = false;

	t_bloque* bloqueViejo = list_remove(pagina->bloques, bloque->indice);
	bloqueViejo->indice = list_size(pagina->bloques) - 1;
	bloqueViejo->metadata->isFree = true;
	bloqueViejo->metadata->size = bloqueViejo->metadata->size - espacio;

	list_add_in_index(pagina->bloques, bloqueNuevo->indice, bloqueNuevo);
	list_add(pagina->bloques, bloqueViejo);

	pagina->tamDisponible = pagina->tamDisponible - espacio;

	int posicion = 0;
	int tope = 0;

	void calcularPosicion(t_bloque* bloque)
	{
		if(bloque->indice <= tope)
		{
			posicion = bloque->metadata->size + 5;
			tope++;
		}
	}

	list_iterate(pagina->bloques,(void*)calcularPosicion);

	//Devuelve una posicion absoluta tomando en cuenta PAGINAS DE STACK, de HEAP y OFFSET de bloques.
	//Es resonsabilidad de las primitivas de CPU sumarle las paginas de codigo.
	t_puntero puntero = ((config.STACK_SIZE + pagina->nro) * tamanioPag) + posicion;

	return puntero;

}

t_puntero alocarMemoria(int espacioSolicitado, int pid)
{
	if(solicitudValida(espacioSolicitado))
	{

		t_pidHeap* elementoPID = getPID(pid);
		if(elementoPID != NULL)
		{

			t_paginaHeap* pagina = getPaginaConEspacio(elementoPID, elementoPID->pid, espacioSolicitado);
			if(pagina != NULL)
			{

				t_bloque* bloque = getBloqueConEspacio(pagina, espacioSolicitado);
				if(bloque != NULL)
				{
					return alocar(pid, espacioSolicitado);

				}else
				{
					//es posible desfragmentar?
					if(true)
					{
						//Es posible desfragmentar
						//TODO:desfragmentar()
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
				return alocar(pid, espacioSolicitado);
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
