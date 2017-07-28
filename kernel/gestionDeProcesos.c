#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <parser/parser.h>
#include <parser/metadata_program.h>
#include <math.h>

#include "gestionDeProcesos.h"
#include "funcionesKernel.h"
#include "deserializador.h"
#include "serializador.h"
#include "estructurasCompartidas.h"
#include "logger.h"


static bool matrizEstados[5][5] = {
//		     		NEW    READY  EXEC   BLOCK  EXIT
		/* NEW 	 */{ false, true, false, false, true },
		/* READY */{ false, false, true, false, true },
		/* EXEC  */{ false, true, false, true, true },
		/* BLOCK */{ false, true, true, false, true },
		/* EXIT  */{ false, false, false, false, false }
};

void transformarCodigoToMetadata(t_proceso* proceso)
{
	t_metadata_program* metadata = metadata_desde_literal(proceso->codigoPrograma);
	proceso->PCB->contadorPrograma = metadata->instruccion_inicio;

	//Etiquetas
	proceso->PCB->indiceEtiquetas = metadata->etiquetas;
	proceso->PCB->etiquetasSize = metadata->etiquetas_size;

	    //Llena indice de codigo
		int i;
		t_sentencia* sentencia;
		for (i = 0; i < metadata->instrucciones_size; i++) {
				sentencia = malloc(sizeof(t_sentencia));
				sentencia->inicio = metadata->instrucciones_serializado[i].start;
				sentencia->fin = sentencia->inicio + metadata->instrucciones_serializado[i].offset;
				list_add(proceso->PCB->indiceCodigo, sentencia);
			}

	free(metadata);
}

t_PCB* crearPCB(t_proceso* proceso, int32_t cantidadDePaginas)
{
	t_PCB* pcb = malloc(sizeof(t_PCB));
	pcb->PID = proceso->pidProceso;
	pcb->contadorPrograma = 0;
	pcb->cantidadPaginas = cantidadDePaginas;
	pcb->exitCode = -1;
	pcb->etiquetasSize = 0;
	pcb->indiceEtiquetas = NULL;
	pcb->stackPointer = stack_crear();
	pcb->indiceCodigo = list_create();

	stack_PCB_main(pcb);

	proceso->PCB = pcb;

	transformarCodigoToMetadata(proceso);

	return pcb;
}

t_proceso* obtenerProceso(int cliente){

	bool criterioProceso(t_proceso* proceso)
	{
		return proceso->CpuDuenio == cliente;
	}

	t_proceso* proceso = list_find(listaDeProcesos, (void*)criterioProceso);
	if (proceso == NULL){
		return NULL;
	}
	return proceso;
}

t_proceso* buscarProcesoPorPID(int PID){

	bool buscarPID(t_proceso* unProceso)
	{
		return unProceso->pidProceso == PID;
	}

	t_proceso* procesoBuscado = list_find(listaDeProcesos,(void*)buscarPID);
	return procesoBuscado;
}

void stack_PCB_main(t_PCB* pcb){

	//Mete el contexto de la funcion main al stack
	t_elemento_stack* stackMain = stack_elemento_crear();
	stackMain->pos = 0;
	stack_push(pcb->stackPointer, stackMain);
}

t_proceso* crearProceso(int pid, int consolaDuenio, char* codigo, int tamanioScript)
{
	printf("Crear proceso - init\n");

	t_proceso* proceso = malloc(sizeof(t_proceso));
	proceso->pidProceso = pid;
	proceso->ConsolaDuenio = consolaDuenio;
	proceso->CpuDuenio = -1;
	proceso->sigusr1 = false;
	proceso->abortado = false;
	proceso->estado = NEW;
	proceso->semaforo = NULL;
	proceso->codigoPrograma = codigo;
	proceso->tamanioScript = tamanioScript;
	proceso->rafagas = 0;
	proceso->rafagasTotales = 0;
	proceso->privilegiadas = 0;
	proceso->cantidadPaginasHeap = 0;
	proceso->cantidadAlocaciones = 0;
	proceso->bytesAlocados = 0;
	proceso->cantidadLiberaciones = 0;
	proceso->bytesLiberados = 0;

	printf("Crear proceso - end\n");

	return proceso;
}

void destructorProcesos(t_proceso* unProceso)
{
	free(unProceso);
}

void finalizarProceso(t_proceso* proceso)
{
	cambiarEstado(proceso,EXIT);

	//liberarRecursosFS(proceso->pidProceso);
	//liberarRecursos(proceso);

	if (proceso->semaforo != NULL){
			t_semaforo* semaforo = dictionary_get(tablaSemaforos,proceso);
			semaforo->colaSemaforo = queue_remove(semaforo->colaSemaforo, proceso->semaforo);
	}

	desasignarCPU(proceso);

}

void bloquearProcesoSem(int cliente, char* semid) {

	if (dictionary_has_key(tablaSemaforos, semid)) {

		t_proceso* proceso = obtenerProceso(cliente);
		bloquearProceso(proceso);
		proceso->semaforo = string_duplicate(semid);

		queue_push(((t_semaforo*) dictionary_get(tablaSemaforos, semid))->colaSemaforo,proceso);
	}
	else{
		//TODO:LOGGEAR ERROR
	}

	//Imprimir colas, opcional: dictionary_iterator(tablaSemaforos,(void*) imprimirColasSemaforos);
}

void bloquearProceso(t_proceso* proceso) {
	//Hacer toda la logica para enviar PCB
	cambiarEstado(proceso,BLOCK);
}

void desbloquearProceso(t_proceso* proceso) {
	//Hacer toda la logica para enviar PCB
	cambiarEstado(proceso,READY);
	proceso->semaforo = NULL;
}

void cambiarEstado(t_proceso* proceso, int estado) {

	bool legalidad;
	legalidad = matrizEstados[proceso->estado][estado];
	if (legalidad) {

		if (estado == BLOCK){
			//queue_push(colaReady, proceso);
		}
		if (estado == READY){
			queue_push(colaReady, proceso);
		}
		else if (estado == EXIT){
			queue_push(colaExit, proceso);
		}

		proceso->estado = estado;
	}
	else{
		exit(EXIT_FAILURE);
	}
}

void rafagaProceso(int cliente){

	t_proceso* proceso = obtenerProceso(cliente);
	proceso->rafagasTotales++;
	planificarExpulsion(proceso);

}

void continuarProceso(t_proceso* proceso) {


	int32_t codAccion = accionContinuarProceso;
	int32_t quantum = config.QUANTUM_SLEEP;
	void* buffer = malloc(2*sizeof(int32_t));
	memcpy(buffer, &codAccion, sizeof(codAccion));
	memcpy(buffer + sizeof(codAccion), &quantum, sizeof(quantum)); //QUANTUM_sleep

    send(proceso->CpuDuenio, buffer, sizeof(codAccion) + sizeof(quantum), 0);

	proceso->rafagas++;

    free(buffer);
}

bool terminoQuantum(t_proceso* proceso) {
	return (bool)(strcmp(config.ALGORITMO, ROUND_ROBIN) == 0 &&
					proceso->rafagas >= config.QUANTUM);
}

void desasignarCPU(t_proceso* proceso) {

	/*Se coloca esta condici{on porque pudo haber pasado que se intente desasignar dos veces.
	proceso->CpuDuenio != -1*/
	if (!proceso->sigusr1 && proceso->CpuDuenio != -1){
		encolarCPU(colaCPU, proceso->CpuDuenio);
	}

	proceso->CpuDuenio = -1;
}

void actualizarPCB(t_proceso* proceso, t_PCB* PCB) { //
	destruir_PCB(proceso->PCB);
	proceso->PCB = PCB;
}

t_PCB* recibirPCBDeCPU(int cpu)
{
	 int tamanio;
	 recv(cpu, &tamanio, sizeof(int), 0);
	 void* pcbSerializado = malloc(tamanio);

	 recv(cpu, pcbSerializado, tamanio, 0);
	 t_PCB* pcb = malloc(sizeof(t_PCB));
	 deserializar_PCB(pcb, pcbSerializado);

	return pcb;
}

void expulsarProceso(t_proceso* proceso) {

	int codAccion = accionDesalojarProceso;
	void* buffer = malloc(sizeof(int));
	memcpy(buffer, &codAccion, sizeof(codAccion)); //CODIGO DE ACCION

	send(proceso->CpuDuenio, buffer, sizeof(codAccion), 0);

	int tamanio;
	recv(proceso->CpuDuenio, &codAccion, sizeof(codAccion), 0);
    recv(proceso->CpuDuenio, &tamanio, sizeof(tamanio), 0);
    void* pcbSerializado = malloc(tamanio);
    recv(proceso->CpuDuenio, pcbSerializado, tamanio, 0);

	t_PCB* pcb = malloc(sizeof(t_PCB));

	deserializar_PCB(pcb, pcbSerializado);

	actualizarPCB(proceso, pcb);

	if (proceso->abortado){
		finalizarProceso(proceso);
	}else if(proceso->estado == EXEC){
		cambiarEstado(proceso, READY);
	}

	desasignarCPU(proceso);

	proceso->rafagas = 0;

	free(pcbSerializado);
}

void liberarRecursos(t_proceso* proceso)
{
	int codAccion = finalizarProgramaAccion;
	int pidParaLiberar = proceso->pidProceso;
	int result;

	void* buffer = malloc(sizeof(int32_t)*3);
	memcpy(buffer, &codAccion, sizeof(codAccion));
	memcpy(buffer + sizeof(codAccion), &pidParaLiberar, sizeof(pidParaLiberar));

	send(memoria, buffer, sizeof(codAccion) + sizeof(pidParaLiberar), 0);
	recv(memoria, &result, sizeof(int32_t), 0);

}

void planificarExpulsion(t_proceso* proceso) {

	bool seLeAcaboElQuantum = terminoQuantum(proceso);


    if(proceso->estado == EXEC && !proceso->abortado)
	{
		if(seLeAcaboElQuantum)
		{
			expulsarProceso(proceso);
		}else
		{
			if(proceso->sigusr1){
				expulsarProceso(proceso);
			}
			else{
				continuarProceso(proceso);
			}
		}
	}
    else if(proceso->estado == BLOCK)
	{
		expulsarProceso(proceso);
	}

	if(proceso->abortado && proceso->estado == READY)
	{
		finalizarProceso(proceso);
	}
	if(proceso->abortado && proceso->estado == EXEC)
	{
		int exitCode= proceso->PCB->exitCode;
		expulsarProceso(proceso);
		proceso->PCB->exitCode = exitCode;
	}


}

void asignarCPU(t_proceso* proceso, int cpu) {

	cambiarEstado(proceso, EXEC);

	proceso->CpuDuenio = cpu;
	proceso->rafagas = 0;
	proceso->sigusr1 = false;
}

void enviarPCBaCPU(t_PCB* pcb, int cpu, int32_t accion)
{
	serializar_PCB(pcb, cpu, accion);
}

void ejecutarProceso(t_proceso* proceso, int cpu) {

	asignarCPU(proceso,cpu);
	enviarPCBaCPU(proceso->PCB, cpu, accionObtenerPCB);
	continuarProceso(proceso);
}

//void recibirFinalizacion(int cliente) {
//	int tamanio;
//	int sigusr1;
//	recv(cliente, &tamanio, sizeof(tamanio), 0);
//	void* pcbSerializado = malloc(tamanio);
//	recv(cliente, pcbSerializado, tamanio, 0);
//	recv(cliente, &sigusr1, sizeof(int), 0);
//
//	t_PCB* pcb = malloc(sizeof(t_PCB));
//	deserializar_PCB(pcb, pcbSerializado);
//	t_proceso* proceso = obtenerProceso(cliente);
//	proceso->PCB = pcb;
//
//	if (proceso != NULL) {
//
//		if(sigusr1 == 1)
//		{
//			proceso->sigusr1 = true;
//		}
//
//		if (!proceso->abortado)
//		{
//			finalizarProceso(proceso);
//		}
//	}
//}

void recibirFinalizacionErronea(int cliente) {

	t_proceso* proceso = obtenerProceso(cliente);

	//Lo aborto por exception
	proceso->abortado = true;
	recibirFinalizacion(cliente);
}


void recibirFinalizacion(int cliente) {

	t_proceso* proceso = obtenerProceso(cliente);
	if (proceso != NULL) {
		proceso->PCB = recibirPCBDeCPU(cliente);
		finalizarProceso(proceso);
		if (!proceso->abortado)
		{
			int32_t codigo = accionConsolaFinalizarNormalmente;
			send(proceso->ConsolaDuenio,&codigo,sizeof(codigo),0);

		}else{
			void* buffer = malloc(sizeof(int32_t)*2);
			int32_t codigo = accionConsolaFinalizarErrorInstruccion;
			memcpy(buffer,&codigo,sizeof(int32_t));
			memcpy(buffer + sizeof(int32_t),&proceso->PCB->exitCode,sizeof(int32_t));
			send(proceso->ConsolaDuenio,buffer,sizeof(int32_t)*2,0);
			free(buffer);
		}
	}
}
/****************************************CONSOLA KERNEL*******************************************************/

void recibirAccionDelUsuarioKernel(int32_t orden)
{
	while(1)
	{
		switch(orden)
		{
			case listadoProcesoCompleto:
						break;

			case totalRafagas:
						break;

			case totalPrivilegiadas:
						break;

			case verTablaArchivosAbiertos:
						break;

			case totalPaginasHeap:
				//Esta accion se subdivide en otras dos
						break;

			case verTablaGlobalArchivos:
						break;

			case modificarGradoMultiprog:
						break;

			case finalizarProcesoDesdeKernel:
						break;

			case detenerPlanificacion:
						break;
		}
	}
}

void modificarGradoDeMultiprogramacion(int nuevoGradoMulti)
{
	//Lei en un ISSUE que se empieza a laburar gradualmente con el nuevo grado.
	//No hay que hacer nada raro como salir a matar procesos si el numero nuevo es menor.
	config.GRADO_MULTIPROG = nuevoGradoMulti;
}

//Funcion nuestra, no de las commons
void queue_iterate(t_queue* self, void (*closure)(void*)) {
	t_link_element *element = self->elements->head;
	while (element != NULL) {
		closure(element->data);
		element = element->next;
	}
}

void imprimirPIDenCola(t_proceso* procesoEnCola){
	char* nueva = string_from_format("PID:%d ",procesoEnCola->PCB->PID);
	string_append(&strCola,nueva);
	free(nueva);
}

void imprimirPIDenLista(t_proceso* procesoEnLista){
	char* nueva = string_from_format("PID:%d ",procesoEnLista->PCB->PID);
	string_append(&strLista, nueva);
	free(nueva);
}

void imprimirColaReady()
{
	strCola = string_new();
	queue_iterate(colaReady, (void*)imprimirPIDenCola);
	log_info(infoLog,"Cola Ready =[%s]",strCola);
	free(strCola);
}

void imprimirColaNew()
{
	strCola = string_new();
	queue_iterate(colaNew, (void*)imprimirPIDenCola);
	log_info(infoLog,"Cola New =[%s]",strCola);
	free(strCola);
}

void imprimirColaBlock()
{
	strCola = string_new();
	queue_iterate(colaBlock, (void*)imprimirPIDenCola);
	log_info(infoLog,"Cola Block =[%s]",strCola);
	free(strCola);
}

void imprimirColaExit()
{
	strCola = string_new();
	queue_iterate(colaExit, (void*)imprimirPIDenCola);
	log_info(infoLog,"Cola Exit =[%s]",strCola);
	free(strCola);
}

void imprimirTodosLosProcesos()
{
	strCola = string_new();
	queue_iterate(listaDeProcesos, (void*)imprimirPIDenCola);
	log_info(infoLog,"Cola Exit =[%s]",strCola);
	free(strCola);
}

void imprimir(t_proceso_estado estado){



	switch(estado)
	{
		case NEW:
			imprimirColaNew();
		break;

		case READY:
			imprimirColaReady();
		break;

		case BLOCK:
			imprimirColaBlock();
		break;

		case EXIT:
			imprimirColaExit();
		break;

		default:
			imprimirTodosLosProcesos();
		break;

	}
}

void rafagasPorProceso(t_proceso* unProceso)
{
	char* rafagas = string_from_format("PID:|%d|, Rafagas: |%d|",
			unProceso->pidProceso,
			unProceso->rafagasTotales);
	string_append(&strRafagas, rafagas);

	free(rafagas);
}

void imprimirRafagas()
{
	strRafagas = string_new();
	list_iterate(listaDeProcesos, (void*)rafagasPorProceso);
	log_info(infoLog,"Rafagas =[%s]", strRafagas);
	free(strRafagas);
}

void privilegiadasPorProceso(t_proceso* unProceso)
{
	char* privilegiadas = string_from_format("PID: |%d|, Privilegiadas: |%d| ",
							unProceso->pidProceso,
							unProceso->privilegiadas);
	string_append(&strRafagas, privilegiadas);
	free(privilegiadas);
}

void imprimirPrivilegiadas()
{
	strPrivilegiadas = string_new();
	list_iterate(listaDeProcesos, (void*)privilegiadasPorProceso);
	log_info(infoLog,"Privilegiadas =[%s]", strPrivilegiadas);
	free(strPrivilegiadas);
}

void alocacionHEAPPorProceso(t_proceso* unProceso)
{
	char* cantidadPaginasHeapAlocar = string_from_format("PID: |%d|, Cant. Paginas: |%d|, Alocar: |%d|, Bytes: |%d| ",
								unProceso->pidProceso,
			                    unProceso->cantidadPaginasHeap,
								unProceso->cantidadAlocaciones,
								unProceso->bytesAlocados);
	string_append(&strPaginasHeapAlocar, cantidadPaginasHeapAlocar);
	free(cantidadPaginasHeapAlocar);
}

void imprimirAlocacionPaginasHeap()
{
	strPaginasHeapAlocar = string_new();
	list_iterate(listaDeProcesos, (void*)alocacionHEAPPorProceso);
	log_info(infoLog,"Paginas Heap =[%s]", strPaginasHeapAlocar);
	free(strPaginasHeapAlocar);
}

void liberacionHEAPPorProceso(t_proceso* unProceso)
{
	char* cantidadPaginasHeapLiberar = string_from_format("PID: |%d|, Cant. Paginas: |%d|, Liberar: |%d|, Bytes: |%d| ",
								unProceso->pidProceso,
			                    unProceso->cantidadPaginasHeap,
								unProceso->cantidadLiberaciones,
								unProceso->bytesLiberados);
	string_append(&strPaginasHeapLiberar, cantidadPaginasHeapLiberar);
	free(cantidadPaginasHeapLiberar);
}

void imprimirLiberacionPaginasHeap()
{
	strPaginasHeapLiberar = string_new();
	list_iterate(listaDeProcesos, (void*)liberacionHEAPPorProceso);
	log_info(infoLog,"Paginas Heap =[%s]", strPaginasHeapLiberar);
	free(strPaginasHeapLiberar);
}

void imprimirTablaGlobalDeArchivos(int pid)
{
	t_proceso* proceso = buscarProcesoPorPID(pid);
	//TODO:LUCAS, hay que traer la tabla de archivos abiertos del proceso
}

