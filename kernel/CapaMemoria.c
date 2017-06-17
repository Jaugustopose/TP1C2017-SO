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
		elemento->pid = pid;
		elemento->paginas = list_create();


	}else
	{
		//Existe
	}
}

void crearBloqueEnPaginaExistente(int nroPagina, int pid, int solicitud)
{


}
