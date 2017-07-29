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
#include "CapaFS.h"


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

	liberarRecursosFS(proceso->pidProceso);
	liberarRecursos(proceso);

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
	if(proceso->CpuDuenio==-1){
		cambiarEstado(proceso,READY);
	}else{
		cambiarEstado(proceso,EXEC);
	}
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

    send(proceso->CpuDuenio, buffer, sizeof(codAccion) + sizeof(quantum), MSG_WAITALL);

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
	 recv(cpu, &tamanio, sizeof(int), MSG_WAITALL);
	 void* pcbSerializado = malloc(tamanio);

	 recv(cpu, pcbSerializado, tamanio, MSG_WAITALL);
	 t_PCB* pcb = malloc(sizeof(t_PCB));
	 deserializar_PCB(pcb, pcbSerializado);

	return pcb;
}

void expulsarProceso(t_proceso* proceso) {

	int codAccion = accionDesalojarProceso;
	void* buffer = malloc(sizeof(int));
	memcpy(buffer, &codAccion, sizeof(codAccion)); //CODIGO DE ACCION

	send(proceso->CpuDuenio, buffer, sizeof(codAccion), MSG_WAITALL);

	int tamanio;
	recv(proceso->CpuDuenio, &codAccion, sizeof(codAccion), MSG_WAITALL);
    recv(proceso->CpuDuenio, &tamanio, sizeof(tamanio), MSG_WAITALL);
    void* pcbSerializado = malloc(tamanio);
    recv(proceso->CpuDuenio, pcbSerializado, tamanio, MSG_WAITALL);

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

	void* buffer = malloc(sizeof(int32_t)*2);
	memcpy(buffer, &codAccion, sizeof(codAccion));
	memcpy(buffer + sizeof(codAccion), &pidParaLiberar, sizeof(pidParaLiberar));

	send(memoria, buffer, sizeof(int32_t)*2, MSG_WAITALL);
	recv(memoria, &result, sizeof(int32_t), MSG_WAITALL);

}

void planificarExpulsion(t_proceso* proceso) {

	bool seLeAcaboElQuantum = terminoQuantum(proceso);


    if(proceso->estado == EXEC && !proceso->abortado)
	{
		if(seLeAcaboElQuantum && !queue_is_empty(colaReady))
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
			send(proceso->ConsolaDuenio,&codigo,sizeof(codigo),MSG_WAITALL);

		}else{
			void* buffer = malloc(sizeof(int32_t)*2);
			int32_t codigo = accionConsolaFinalizarErrorInstruccion;
			memcpy(buffer,&codigo,sizeof(int32_t));
			memcpy(buffer + sizeof(int32_t),&proceso->PCB->exitCode,sizeof(int32_t));
			send(proceso->ConsolaDuenio,buffer,sizeof(int32_t)*2,MSG_WAITALL);
			free(buffer);
		}
	}
}
/****************************************CONSOLA KERNEL*******************************************************/



void modificarGradoDeMultiprogramacion()
{
	puts("Ingrese el nuevo grado de multiprogramación deseado");
	char input[10];
	if (fgets(input, sizeof(input), stdin) == NULL) {
		log_error(errorLog, "Error al leer consola del Kernel! - input: %s", input);
		return;
	}
	char* eptr;
	int nuevoGradoMulti = strtol(input, &eptr, 10);
	if (nuevoGradoMulti == 0 && input == eptr) {
		log_error(errorLog, "Error con el valor ingresado - input: %s", input);
		return;
	}

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
	log_info(infoLog,"En estado Ready =[%s]",strCola);
	free(strCola);
}

void imprimirColaNew()
{
	strCola = string_new();
	queue_iterate(colaNew, (void*)imprimirPIDenCola);
	log_info(infoLog,"En estado New =[%s]",strCola);
	free(strCola);
}

void imprimirColaExec()
{
	strLista = string_new();
	bool _estaEnExec(t_proceso *p) {
		return p->estado == EXEC;
	}
	t_list* listaBlock = list_filter(listaDeProcesos, (void*)_estaEnExec);
	list_iterate(listaBlock, (void*)imprimirPIDenCola);
	log_info(infoLog,"En estado Exec =[%s]",strLista);
	free(strLista);
}

void imprimirColaBlock()
{
	strLista = string_new();
	bool _estaEnExec(t_proceso *p) {
		return p->estado == BLOCK;
	}
	t_list* listaBlock = list_filter(listaDeProcesos, (void*)_estaEnExec);
	list_iterate(listaBlock, (void*)imprimirPIDenCola);
	log_info(infoLog,"En estado Block =[%s]",strLista);
	free(strLista);
}

void imprimirColaExit()
{
	strCola = string_new();
	queue_iterate(colaExit, (void*)imprimirPIDenCola);
	log_info(infoLog,"En estado Exit =[%s]",strCola);
	free(strCola);
}

void imprimirTodosLosProcesos()
{
	strCola = string_new();
	list_iterate(listaDeProcesos, (void*)imprimirPIDenCola);
	log_info(infoLog,"Listado completo de procesos =[%s]",strCola);
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

		case EXEC:
			imprimirColaExec();
		break;

		case BLOCK:
			imprimirColaBlock();
		break;

		case EXIT:
			imprimirColaExit();
		break;

		case ALL:
			imprimirTodosLosProcesos();
		break;

	}
}
void escucharConsolaKernel() {
	log_info(infoLog, "Escuchando nuevas solicitudes de consola en nuevo hilo");

		while (1) {
			puts("Ingrese una acción a realizar\n");
			puts("1: Listar Procesos");
			puts("2: Operaciones sobre un proceso");
			puts("3: Obtener tabla global de archivos");
			puts("4: Modificar grado de multiprogramación");
			puts("5: Finalizar proceso");
			puts("6: Detener planificación");
			char accion[3];
			if (fgets(accion, sizeof(accion), stdin) == NULL) {
				printf("Error al leer la consola !\n");
				log_error(errorLog, "ERROR AL LEER LA CONSOLA! - accion: %s", accion);
				return;
			}
			int codAccion = accion[0] - '0';
			switch (codAccion) {
			case listarProcesos:
				puts("\n¿Qué desea listar?");
				puts("0: Procesos en NEW");
				puts("1: Procesos en READY");
				puts("2: Procesos en EXEC");
				puts("3: Procesos en BLOCK");
				puts("4: Procesos en EXIT");
				puts("5: Todos los procesos\n");
				char input[10];
				if (fgets(input, sizeof(input), stdin) == NULL) {
					log_error(errorLog, "Error al leer consola del Kernel! - input: %s", input);
					break;
				}
				char* eptr;
				int inputInt = strtol(input, &eptr, 10);
				if (inputInt == 0 && input == eptr) {
					log_error(errorLog, "Error con el valor ingresado - input: %s", input);
					break;
				}
				imprimir(inputInt);
				break;
			case operarSobreProceso:
				puts("\nIngrese el número de PID:");
				char inputProceso[10];
				if (fgets(inputProceso, sizeof(inputProceso), stdin) == NULL) {
					log_error(errorLog, "Error al leer consola del Kernel! - input: %s", inputProceso);
					break;
				}
				char* ptrInput;
				int pidBuscado = strtol(inputProceso, &ptrInput, 10);
				if (pidBuscado == 0 && inputProceso == ptrInput) {
					log_error(errorLog, "Error con el valor ingresado - input: %s", inputProceso);
					break;
				}
				int _esProcesoBuscado(t_proceso* proceso)
				{
					return proceso->pidProceso == pidBuscado;
				}
				t_proceso* proceso = list_find(listaDeProcesos, (void*)_esProcesoBuscado);
				if (proceso == NULL) {
					log_warning(warningLog, "Proceso no encontrado!");
				} else {
					realizarOperacionSobreProceso(proceso);
				}
				break;
			case obtenerTGArchivos:
				printf("Codificar obtenerTGArchivos!\n");
				break;
			case modificarMultiprogramacion:
				modificarGradoDeMultiprogramacion();
				break;
			case finalizarProcesoPorUsuario:
				printf("Codificar finalizarProcesoPorUsuario!\n");
				break;
			case detenerPlanificacion:

				planificacionDetenida = 1;
				break;
			default:
				printf("No se reconece la acción %d!\n", codAccion);
			}
		}
}
void realizarOperacionSobreProceso(t_proceso* proceso)
{
	puts("\nOperaciones:");
	puts("1: Obtener cantidad de ráfagas ejecutadas");
	puts("2: Obtener cantidad de operaciones privilegiadas ejecutadas");
	puts("3: Obtener la tabla de archivos abiertos por el proceso");
	puts("4: Obtener cantidad de páginas de Heap utilizadas");
	puts("5: Obtener cantidad de syscalls ejecutadas\n");
	char input[10];
	if (fgets(input, sizeof(input), stdin) == NULL) {
		log_error(errorLog, "Error al leer consola del Kernel! - input: %s", input);
		return;
	}
	char* ptrInput;
	int operacion = strtol(input, &ptrInput, 10);
	if (operacion == 0) {
		log_error(errorLog, "Error con el valor ingresado - input: %s", input);
		return;
	}
	switch(operacion)
	{
		case 1:
			rafagasPorProceso(proceso);
			break;

		case 2:
			privilegiadasDelProceso(proceso);
			break;

		case 3:
			printf("Tabla de archivos abiertos por el proceso!\n");
			break;

		case 4:
			obtenerInfoHeapParaProceso(proceso);
			break;

		case 5:
			break;

		default:
			log_error(errorLog, "Operación no reconocida!");
	}
}

void obtenerInfoHeapParaProceso(t_proceso* proceso)
{
	puts("\nOpciones:");
	puts("1: Obtener cantidad de acciones alocar realizadas");
	puts("2: Obtener cantidad de acciones liberar realizadas");
	char input[10];
	if (fgets(input, sizeof(input), stdin) == NULL) {
		log_error(errorLog, "Error al leer consola del Kernel! - input: %s", input);
		return;
	}
	char* ptrInput;
	int opcion = strtol(input, &ptrInput, 10);
	if (opcion == 0) {
		log_error(errorLog, "Error con el valor ingresado - input: %s", input);
		return;
	}
	switch(opcion)
	{
		case 1:
			alocacionHEAPPorProceso(proceso);
			break;

		case 2:
			liberacionHEAPPorProceso(proceso);
			break;

		default:
			log_error(errorLog, "Opción no reconocida!");
	}
}

void rafagasPorProceso(t_proceso* unProceso)
{
	char* rafagas = string_from_format("PID:|%d|, Rafagas: |%d|",
			unProceso->pidProceso,
			unProceso->rafagasTotales);
	log_info(infoLog,"[%s]", rafagas);

	free(rafagas);
}

void privilegiadasDelProceso(t_proceso* unProceso)
{
	char* privilegiadas = string_from_format("PID: |%d|, Privilegiadas: |%d| ", unProceso->pidProceso, unProceso->privilegiadas);
	log_info(infoLog,"[%s]", privilegiadas);
	free(privilegiadas);
}

void alocacionHEAPPorProceso(t_proceso* unProceso)
{
	char* cantidadPaginasHeapAlocar = string_from_format("PID: |%d|, Cant. Paginas: |%d|, Alocar: |%d|, Bytes: |%d| ",
								unProceso->pidProceso,
			                    unProceso->cantidadPaginasHeap,
								unProceso->cantidadAlocaciones,
								unProceso->bytesAlocados);
	log_info(infoLog,"[%s]", cantidadPaginasHeapAlocar);
	free(cantidadPaginasHeapAlocar);
}

void liberacionHEAPPorProceso(t_proceso* unProceso)
{
	char* cantidadPaginasHeapLiberar = string_from_format("PID: |%d|, Cant. Paginas: |%d|, Liberar: |%d|, Bytes: |%d| ",
								unProceso->pidProceso,
			                    unProceso->cantidadPaginasHeap,
								unProceso->cantidadLiberaciones,
								unProceso->bytesLiberados);
	log_info(infoLog,"[%s]", cantidadPaginasHeapLiberar);
	free(cantidadPaginasHeapLiberar);
}

void imprimirTablaGlobalDeArchivos(int pid)
{
	//t_proceso* proceso = buscarProcesoPorPID(pid);
	imprimirArchivosPid(pid);
}

