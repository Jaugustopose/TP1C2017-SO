/*
 * funcionesKernel.h
 *
 *  Created on: 24/7/2017
 *      Author: utnso
 */

#ifndef FUNCIONESKERNEL_H_
#define FUNCIONESKERNEL_H_

//Prototipos
int redondear(float numero);
void encolarCPU(t_queue* cola, int socket);
int desencolarCPU(t_queue* cola);
t_queue* queue_remove(t_queue* queue, int toRemove);



#endif /* FUNCIONESKERNEL_H_ */
