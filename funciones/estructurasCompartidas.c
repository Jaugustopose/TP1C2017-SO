
#include "estructurasCompartidas.h"



/***********OPERACIONES STACK*************************/

t_stack* stack_crear() {
	t_list* stack = list_create();
	return stack;
}

t_elemento_stack* stack_elemento_crear(){
	t_elemento_stack* elem = malloc(sizeof(t_elemento_stack));
	elem->argumentos = list_create();
	elem->identificadores = dictionary_create();

	return elem;
}

void stack_elemento_destruir(t_elemento_stack* elem){
	list_destroy(elem->argumentos);
	dictionary_destroy(elem->identificadores);
	free(elem);
}

void stack_destruir(t_stack* stack){
	list_iterate((t_list*)stack,(void*)stack_elemento_destruir);
	list_destroy((t_list*)stack);
}

t_elemento_stack* stack_obtener(t_stack* stack, int indice){
	return (t_elemento_stack*)list_get((t_list*)stack,indice);
}

t_elemento_stack* stack_head(t_stack* stack){
	return stack_obtener(stack, stack_tamanio(stack)-1);
}

t_elemento_stack* stack_pop(t_stack* stack) {
	return (t_elemento_stack*)list_remove((t_list*)stack, stack_tamanio(stack)-1);
}

void stack_push(t_stack* stack, t_elemento_stack* elem) {
	list_add((t_list*)stack, (void*)elem);
}

int stack_tamanio_memoria(t_stack* stack) {
	int i = 0;
	int bytes = 0;
	for (i = 0; i < stack_tamanio(stack); i++) {
	t_elemento_stack* elem = stack_obtener(stack, i);
	bytes = bytes + (list_size(elem->argumentos) + dictionary_size(elem->identificadores))* sizeof(int);
	}
	return bytes;
}

int stack_tamanio(t_stack* stack){
	return list_size(stack);
}

//t_pedido* stack_proximo_pedido(t_stack* stack, int tamanioPagina) {
//
//	t_pedido* pedido = malloc(sizeof(t_pedido));
//	int tamanioActualDeStack = stack_tamanio_memoria(stack);
//	pedido->nroPagina = tamanioActualDeStack / tamanioPagina;
//	pedido->offset = tamanioActualDeStack - (pedido->nroPagina * tamanioPagina);
//	pedido->size = sizeof(int);
//	return pedido;
//}

//ATENCION: SE TOMAN LAS DIRECCIONES LOGICAS DESDE LA PAGINA 1 DEL CODIGO + STACK + HEAP
t_pedido* stack_proximo_pedido(t_stack* stack, int tamanioPagina, int cantidadPaginasCodigo) {

	t_pedido* pedido = malloc(sizeof(t_pedido));
	int tamanioActualDeStack = stack_tamanio_memoria(stack);
	pedido->nroPagina = (cantidadPaginasCodigo + tamanioActualDeStack) / tamanioPagina;
	pedido->offset = (cantidadPaginasCodigo + tamanioActualDeStack) - (pedido->nroPagina * tamanioPagina);
	pedido->size = sizeof(int);
	return pedido;
}

void destruir_PCB(t_PCB* pcb){
	//Lo usa CPU y Kernel
	list_destroy(pcb->indiceCodigo);
	stack_destruir(pcb->stackPointer);

	free(pcb);
}


char* leerTamanioYMensaje(int sock){
	char* serialTamanio = malloc(sizeof(int32_t));
	recv(sock, serialTamanio, sizeof(int32_t), 0);
	int32_t tamanio = char4ToInt(serialTamanio);
	char* mensaje = malloc(tamanio);
	recv(sock, mensaje, tamanio, 0);
	free(serialTamanio);
	return mensaje;
}

void enviarTamanioYString(int codigoAccion, int sock, char* mensaje){
	enviarTamanioYSerial(codigoAccion, sock, strlen(mensaje)+1, mensaje);
}
void enviarTamanioYSerial(int codigoAccion, int sock, int tamanio, char* mensaje){
	int offset = 0;

	char* bufferVarComp = malloc(sizeof(int) + sizeof(int) + sizeof(tamanio));
	memcpy(bufferVarComp, &codigoAccion, sizeof(codigoAccion));
	offset += sizeof(codigoAccion);
	memcpy(bufferVarComp + offset, &tamanio, sizeof(int));
	offset += sizeof(tamanio);
	memcpy(bufferVarComp + offset, mensaje, tamanio);
	offset += tamanio;

	send(sock, bufferVarComp, offset, 0);
}
