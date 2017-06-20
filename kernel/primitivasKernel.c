#include "kernel.h"

void primitivaSignal(int cliente, char* semaforoID)
{
	t_semaforo* semaforo = (t_semaforo*)dictionary_get(tablaSemaforos, semaforoID);

		if (!queue_is_empty(semaforo->colaSemaforo)) {

			t_proceso* proceso = queue_pop(semaforo->colaSemaforo);
			//TODO: IMPLEMENTAR! desbloquearProceso(proceso);
		}
		else{
			semaforo->valorSemaforo++;
		}
}

void primitivaWait(int cliente, char* semaforoID)
{
	t_semaforo* semaforo = (t_semaforo*)dictionary_get(tablaSemaforos, semaforoID);

	if (semaforo->valorSemaforo > 0){
		semaforo->valorSemaforo--;
		}
	else{
		//TODO: IMPLEMENTAR! bloquearProcesoSem(cliente, semaforoID);
	}
}

int primitivaDevolverCompartida()
{

}

void primitivaAsignarCompartida()
{

}

void imprimir()
{

}

void recibirPedidoMemoria(int tamanioSolicitud, int pidSolicitante)
{
	if(peticion_valida(tamanioSolicitud))
	{
		//pedido a memoria de una pagina: cantpaginas = 1, pidSolicitante
		//recibe respuesta
	//	crearPagina(pidSolicitante, tamanioSolicitud);


	}else
	{
		//finalizar programa abruptamente por exceder tamanio del pedido
	}
}

void obtenerValorCompartida()
{
	char* bufferVarComp;
	recv(fdCliente, bufferVarComp, sizeof(int32_t), 0);
	int tamanio = (int)*bufferVarComp;
	char* nombreVariable = malloc(tamanio);
	recv(fdCliente, nombreVariable, tamanio, 0);

	char* nombre = nombreVariable;
	int valorPrueba = 172;


	void* resultado = malloc(sizeof(int32_t));
	memcpy(resultado, &valorPrueba, sizeof(int32_t));
	send(fdCliente, resultado, sizeof(int32_t), 0);
}
