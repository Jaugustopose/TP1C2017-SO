#include "gestionDeProcesos.h"


static bool matrizEstados[5][5] = {
//		     		NEW    READY  EXEC   BLOCK  EXIT
		/* NEW 	 */{ false, true, false, false, true },
		/* READY */{ false, false, true, false, true },
		/* EXEC  */{ false, true, false, true, true },
		/* BLOCK */{ false, true, true, false, true },
		/* EXIT  */{ false, false, false, false, false }
};

//Nuestra. Dada una cola, te saca un elemento y te devuelve una nueva sin ese elemento
t_queue* queue_remove(t_queue* queue, void* toRemove){
	t_queue* queueNew = queue_create();
	while(!queue_is_empty(queue)){
		void* data = queue_pop(queue);
		if (data!=toRemove)
			queue_push(queueNew,data);
	}
	queue_destroy(queue);
	return queueNew;
}

void transformarCodigoToMetadata(t_PCB* pcb, char* cod)
{
	t_metadata_program* metadata = metadata_desde_literal(cod);
	pcb->contadorPrograma = metadata->instruccion_inicio;

	//Etiquetas
	pcb->indiceEtiquetas = metadata->etiquetas;
	pcb->etiquetasSize = metadata->etiquetas_size;

	    //Llena indice de codigo
		int i;
		t_sentencia* sentencia;
		for (i = 0; i < metadata->instrucciones_size; i++) {
				sentencia = malloc(sizeof(t_sentencia));
				sentencia->inicio = metadata->instrucciones_serializado[i].start;
				sentencia->fin = sentencia->inicio + metadata->instrucciones_serializado[i].offset;
				list_add(pcb->indiceCodigo, sentencia);
			}

	free(metadata);
}

t_PCB* crearPCB()
{
	t_PCB* pcb = malloc(sizeof(t_PCB));

	pcb->PID=0;
	pcb->contadorPrograma = 0;
	pcb->cantidadPaginas = 0;
	pcb->exitCode = -1;
	pcb->etiquetasSize = 0;
	pcb->indiceEtiquetas = NULL;
	pcb->stackPointer = stack_crear();
	pcb->indiceCodigo = list_create();

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

void stack_PCB_main(t_PCB* pcb){

	//Mete el contexto de la funcion main al stack
	t_elemento_stack* main = stack_elemento_crear();
	main->pos = 0;
	stack_push(pcb->stackPointer, main);
}

t_proceso* crearProceso(int pid, int consolaDuenio, char* codigo)
{

	printf("Crear proceso - init\n");
	t_proceso* proceso = malloc(sizeof(t_PCB));
	proceso->PCB = crearPCB();
	proceso->PCB->PID = pid;
	proceso->ConsolaDuenio = consolaDuenio;
	proceso->CpuDuenio = -1;
	proceso->sigusr1 = false;
	proceso->abortado = false;
	proceso->estado = NEW;
	proceso->semaforo = NULL;
	printf("Crear proceso - end\n");

	transformarCodigoToMetadata(proceso->PCB, codigo);

	stack_PCB_main(proceso->PCB);

	//cambiarEstado(proceso, READY);

	return proceso;
}

void destructorProcesos(t_proceso* unProceso)
{
	free(unProceso);
}

void finalizarProceso(t_proceso* proceso)
{
	cambiarEstado(proceso,EXIT);

	//TODO: ELIMINAR ARCHIVOS ABIERTOS QUE TENGAMOS!!!!

	if (proceso->semaforo != NULL){
			t_semaforo* semaforo = dictionary_get(tablaSemaforos,proceso);
			semaforo->colaSemaforo = queue_remove(semaforo->colaSemaforo, proceso->semaforo);
	}

	desasignarCPU(proceso);

	bool esElProcesoBuscado(t_proceso* unProceso)
	{
		return unProceso->PCB->PID == proceso->PCB->PID;
	}

	//Lo saco de la lista de procesos. Queda su PCB en la cola de exit para historico
	list_remove_and_destroy_by_condition(listaDeProcesos,(void*)esElProcesoBuscado, (void*)destructorProcesos);
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
	cambiarEstado(proceso,BLOCK);
}

void desbloquearProceso(t_proceso* proceso) {

	cambiarEstado(proceso,READY);
	proceso->semaforo = NULL;
}

void cambiarEstado(t_proceso* proceso, int estado) {

	bool legalidad;
	legalidad = matrizEstados[proceso->estado][estado];
	if (legalidad) {

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
	proceso->rafagas++;
	planificarExpulsion(proceso);
}

void continuarProceso(t_proceso* proceso) {

	int codAccion = accionContinuarProceso;
	int quantum = config.QUANTUM_SLEEP;
	void* buffer = malloc(2*sizeof(int));
	memcpy(buffer, &codAccion, sizeof(codAccion)); //CODIGO DE ACCION
	memcpy(buffer + sizeof(codAccion), &quantum, sizeof(quantum)); //QUANTUM_sleep

    send(proceso->CpuDuenio, buffer, sizeof(codAccion) + sizeof(quantum), 0);
    free(buffer);
}

bool terminoQuantum(t_proceso* proceso) {
	return (proceso->rafagas > config.QUANTUM);
}

void desasignarCPU(t_proceso* proceso) {
	if (!proceso->sigusr1){
		queue_push(colaCPU, (void*)proceso->CpuDuenio);
	}

	proceso->CpuDuenio = -1;
}

void actualizarPCB(t_proceso* proceso, t_PCB* PCB) { //
	destruir_PCB(proceso->PCB);
	proceso->PCB = PCB;
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

		if (!proceso->abortado && proceso->estado == EXEC){
			cambiarEstado(proceso, READY);
		}
	desasignarCPU(proceso);

	free(pcbSerializado);
}

void planificarExpulsion(t_proceso* proceso) {

	if(proceso->estado == BLOCK || (config.ALGORITMO == ROUND_ROBIN && terminoQuantum(proceso))) {
	    expulsarProceso(proceso);
	   return;
	 }

	if((proceso->estado == EXEC && (config.ALGORITMO == ROUND_ROBIN && terminoQuantum(proceso))) || proceso->abortado)
	{
		expulsarProceso(proceso);
	}
	else{
		continuarProceso(proceso);
	}
	if(proceso->abortado)
	{
		//TODO: LIBERAR RECURSOS, FINALIZAR PROCESO Y CONSOLA ASOCIADA
	}

}

void asignarCPU(t_proceso* proceso, int cpu) {
	cambiarEstado(proceso, EXEC);
	proceso->CpuDuenio = cpu;
	proceso->rafagas = 0;
	proceso->sigusr1 = false;
}

void ejecutarProceso(t_proceso* proceso, int cpu) {

	asignarCPU(proceso,cpu);
	enviarPCBaCPU(proceso->PCB, cpu, accionObtenerPCB);
	continuarProceso(proceso);
}

void recibirFinalizacion(int cliente) {
	t_proceso* proceso = obtenerProceso(cliente);
	if (proceso != NULL) {
		if (!proceso->abortado)
		{
			finalizarProceso(proceso);
		}
	}
}
