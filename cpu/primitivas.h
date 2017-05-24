
#ifndef PRIMITIVAS_H_
#define PRIMITIVAS_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <commons/config.h>
#include <commons/string.h>
#include <parser/parser/parser.h>
#include <parser/parser/metadata_program.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>

AnSISOP_funciones funciones;
AnSISOP_kernel  funcionesKernel;


t_puntero definir_variable(t_nombre_variable variable);
t_puntero obtener_posicion_de(t_nombre_variable variable);

void inicializarPrimitivas();

#endif /* PRIMITIVAS_H_ */
