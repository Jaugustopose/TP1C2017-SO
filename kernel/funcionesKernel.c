/*
 * funcionesKernel.c
 *
 *  Created on: 24/7/2017
 *      Author: utnso
 */
#include <commons/collections/queue.h>
#include <stdlib.h>

int redondear(float numero) {
		int resultado;
		if((numero - (int)numero) !=0){
			numero++;
			resultado = (int) numero;
		}else {
			resultado = (int)numero;
		}

		printf("%d\n",resultado);
		return resultado;
}

void encolarCPU(t_queue* cola, int socket){
	int *p = malloc(sizeof(int));
	*p = socket;
	queue_push(cola, p);
}

int desencolarCPU(t_queue* cola){
	int *p = queue_pop(cola);
	int socket = *p;
	free(p);
	return socket;
}

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


