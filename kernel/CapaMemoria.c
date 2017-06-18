#include "CapaMemoria.h"

bool peticion_valida(int tamanio)
{
	return (tamanioPag - 10) <= tamanio;
}

int tamanioDisponibleDeLaPagina(int tamanioDispActual, int espacioPedido)
{
	return (tamanioDispActual - 10 - espacioPedido);
}

bool existePid(t_pidHeap* elem)
{
	return elem;
}

void crearPagina()
{

}

void buscarPIDHeap(int pid, int solicitud)
{
	t_pidHeap* elemento = list_find(tablaPaginasHeap, (void*)existePid);

	if(elemento == NULL)
	{
		//Crea la nueva pagina
		elemento = malloc(sizeof(t_pidHeap));
		elemento->pid = pid;
		elemento->paginas = list_create();

		t_paginaHeap* pagina = malloc(sizeof(t_paginaHeap));
		pagina->bloques = list_create();

		t_bloqueHeap* bloque = malloc(sizeof(t_bloqueHeap));
		heapMetadata* metadata = malloc(sizeof(uint32_t) + sizeof(_Bool));
		metadata->size = tamanioPag;
		metadata->isFree = true;
		char* data;

		bloque->metadata = metadata;
		bloque->data = data;

		list_add(pagina->bloques, bloque);
		list_add(elemento->paginas, pagina);

		//Envia Solicitud de paginas HEAP
		int offset = 0;
		int codigoAccion = solicitarPaginasAccion;
		char* bufferMemoria = malloc(sizeof(codigoAccion) + sizeof(pedidoSolicitudPaginas_t));

		pedidoSolicitudPaginas_t pedido;
		pedido.pid = pid;
		pedido.cantidadPaginas = 1;

		memcpy(bufferMemoria, &codigoAccion, sizeof(codigoAccion));
		offset += sizeof(codigoAccion);
		memcpy(bufferMemoria + offset , &pedido, sizeof(pedidoSolicitudPaginas_t));
		offset += sizeof(pedidoSolicitudPaginas_t);

		send(memoria, bufferMemoria, offset, 0);

		char* resultadoBuffer = malloc(sizeof(int));
		recv(memoria,resultadoBuffer,sizeof(int),0);
		int* resultado;
		deserializar_int(&resultado, resultadoBuffer);

		//interpretar resultado

		free(bufferMemoria);
		free(resultadoBuffer);


	}else
	{
		//Existe
	}
}

void crearBloqueEnPaginaExistente(int nroPagina, int pid, int solicitud)
{


}
