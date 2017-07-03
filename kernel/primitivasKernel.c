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

void obtenerValorCompartida()
{
	char* compartidaSerial = leerTamanioYMensaje(fdCliente);
	char* compartida = string_from_format("!%s",compartidaSerial);
	char* valor = intToChar4(devolverCompartida(compartida));

	send(fdCliente, valor, sizeof(int32_t), 0);

	 //t_proceso* proceso = obtenerProceso(fdCliente);
	 //if (!dictionary_has_key(tablaCompartidas,compartida))
	//proceso->abortado=true;

	free(valor);
	free(compartidaSerial);
	free(compartida);

	//clientes[fdCliente].atentido=false;
}

void obtenerAsignarCompartida(){
	char* compartidaSerial = leerTamanioYMensaje(fdCliente);
	char* compartida = string_from_format("!%s",compartidaSerial);
	char* valor = malloc(sizeof(int));
	recv(fdCliente, valor, sizeof(int), 0);
	asignarCompartida(compartida, char4ToInt(valor));

	//	t_proceso* proceso = obtenerProceso(fdCliente);
	//	if (!dictionary_has_key(tablaCompartidas,compartida))
	//		proceso->abortado=true;


	//clientes[fdCliente].atentido=false;
	int valorAsignado = devolverCompartida(compartida);
	send(fdCliente, &valorAsignado,sizeof(int),0);
	free(compartida);
	free(compartidaSerial);
	free(valor);
}

int devolverCompartida(char* compartida) {
	if (dictionary_has_key(tablaCompartidas,compartida))
	{
		return (*(int*)dictionary_get(tablaCompartidas, compartida));
	}
	else{
		//ERROR: Se solicita el valor de una compartida inexistente
	}
	return 0;
}

void asignarCompartida(char* compartida, int valor) {
	if (dictionary_has_key(tablaCompartidas,compartida))
	{
		*(int*)dictionary_get(tablaCompartidas, compartida) = valor;
	}
	else{
		//ERROR: Se solicito asignar un valor a  una compartida inexistente
	}
}

void primitivaReservar(int espacioSolicitado)
{
//TODO:IMPLEMENTAR MANEJO HEAP

}

void primitivaLiberar(int puntero)
{
//TODO:IMPLEMENTAR MANEJO HEAP
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
