#include "kernel.h"
#include "gestionDeProcesos.h"

void primitivaWait(int cliente, char* semaforoID);
void primitivaSignal(int cliente, char* semaforoID);
int devolverCompartida(char* compartida);
int asignarCompartida(char* compartida, int valor, int cliente);

void recibirWait(int cliente)
{
	void* serialTamanio = malloc(sizeof(int32_t));
	recv(cliente, serialTamanio, sizeof(int32_t), 0);
	int32_t tamanio = char4ToInt(serialTamanio);
	char* semid = malloc(tamanio);
	recv(cliente, semid, tamanio, 0);

	primitivaWait(cliente, semid);
	free(semid);
}

void recibirSignal(int cliente)
{
	void* serialTamanio = malloc(sizeof(int32_t));
	recv(cliente, serialTamanio, sizeof(int32_t), 0);
	int32_t tamanio = char4ToInt(serialTamanio);
	char* semid = malloc(tamanio);
	recv(cliente, semid, tamanio, 0);

	primitivaSignal(cliente, semid);
	free(semid);
}

void primitivaSignal(int cliente, char* semaforoID)
{
	t_proceso* proceso = obtenerProceso(cliente);
	proceso->privilegiadas++;

	t_semaforo* semaforo = (t_semaforo*)dictionary_get(tablaSemaforos, semaforoID);

		if (!queue_is_empty(semaforo->colaSemaforo)) {

			t_proceso* proceso = queue_pop(semaforo->colaSemaforo);
		    desbloquearProceso(proceso);
		}
		else{
			semaforo->valorSemaforo++;
		}

}

void primitivaWait(int cliente, char* semaforoID)
{
	t_proceso* proceso = obtenerProceso(cliente);
	proceso->privilegiadas++;

	t_semaforo* semaforo = (t_semaforo*)dictionary_get(tablaSemaforos, semaforoID);

	if (semaforo->valorSemaforo > 0){
		semaforo->valorSemaforo--;
	}else{
		bloquearProcesoSem(cliente, semaforoID);
	}
}

void obtenerValorCompartida(int cliente)
{

	char* serialTamanio = malloc(sizeof(int32_t));
	recv(cliente, serialTamanio, sizeof(int32_t), 0);
	int32_t tamanio = char4ToInt(serialTamanio);
	void* compartidaSerial = malloc(tamanio);
	recv(cliente, compartidaSerial, tamanio, 0);
	char* compartida = string_from_format("!%s",compartidaSerial);
	int valor = devolverCompartida(compartida);

	t_proceso* proceso = obtenerProceso(cliente);
	proceso->privilegiadas++;

	if(valor != -1)
	{
		char* valorSerial = intToChar4(valor);
		send(fdCliente, valorSerial, sizeof(int32_t), 0);
	}else
	{
		//ERROR:ABORTAR
	}

	free(compartidaSerial);
	//clientes[fdCliente].atentido=false;
}

void obtenerAsignarCompartida(int cliente){

	char* serialTamanio = malloc(sizeof(int32_t));
	recv(cliente, serialTamanio, sizeof(int32_t), 0);
	int32_t tamanio = char4ToInt(serialTamanio);
	void* compartidaSerial = malloc(tamanio);
	recv(cliente, compartidaSerial, tamanio, 0);
	char* compartida = string_from_format("!%s",compartidaSerial);
	int32_t valorNuevo;
	recv(cliente, &valorNuevo, sizeof(int32_t), 0);
	int32_t resultado = asignarCompartida(compartida, valorNuevo, cliente);

	t_proceso* proceso = obtenerProceso(cliente);
	proceso->privilegiadas++;

	if(resultado != -1)
	{
		int valorAsignado = devolverCompartida(compartida);
		send(fdCliente, &valorAsignado,sizeof(int),0);
	}else
	{
		//ERROR:ABORTAR
	}

	free(compartidaSerial);
}

int devolverCompartida(char* compartida) {
	if (dictionary_has_key(tablaCompartidas,compartida))
	{
		return (*(int*)dictionary_get(tablaCompartidas, compartida));
	}
	else{
		//ERROR: Se solicita el valor de una compartida inexistente
		 t_proceso* proceso = obtenerProceso(fdCliente);
		 if (!dictionary_has_key(tablaCompartidas,compartida))
		 {
			proceso->abortado=true;
		 }

		return -1;
	}

}

int asignarCompartida(char* compartida, int valor, int cliente) {
	if (dictionary_has_key(tablaCompartidas,compartida))
	{
		*(int*)dictionary_get(tablaCompartidas, compartida) = valor;
		return valor;
	}
	else{
		//ERROR: Se solicito asignar un valor a  una compartida inexistente

		t_proceso* proceso = obtenerProceso(cliente);
		if(!dictionary_has_key(tablaCompartidas,compartida))
		{
		    proceso->abortado=true;
		}

		return -1;
		//clientes[fdCliente].atentido=false;
	}
}

void atenderSolicitudMemoriaDinamica()
{
	int espacioSolicitado;
	int pid;

	recv(fdCliente, &pid, sizeof(int),0);
	recv(fdCliente, &espacioSolicitado, sizeof(int),0);

	t_proceso* proceso = buscarProcesoPorPID(pid);
	proceso->privilegiadas++;

	int puntero = alocarMemoria(espacioSolicitado, pid);

	send(fdCliente, &puntero, sizeof(int), 0);
}

void atenderLiberacionMemoriaDinamica()
{
	int punteroRecibido;
	int pid;
	int cantPaginasCodigo;
	recv(fdCliente, &pid, sizeof(int),0);
	recv(fdCliente, &punteroRecibido, sizeof(int),0);
	recv(fdCliente, &cantPaginasCodigo, sizeof(int),0);

	t_proceso* proceso = buscarProcesoPorPID(pid);
	proceso->privilegiadas++;

	int result = liberarMemoria(punteroRecibido, pid, cantPaginasCodigo);
	send(fdCliente, &result, sizeof(int32_t),0);
}
