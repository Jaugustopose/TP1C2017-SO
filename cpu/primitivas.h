
#ifndef PRIMITIVAS_H_
#define PRIMITIVAS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <commons/config.h>
#include <commons/string.h>
//#include <parser/parser.h>
//#include <parser/metadata_program.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include "estructurasCompartidas.h"
#include "cpu.h"

AnSISOP_funciones funciones;
AnSISOP_kernel  funcionesKernel;

enum tipoVariable
{
	DECLARADA = 1,
	PARAMETRO = 2,
	NOEXISTE = 3
};


bool esParametro(t_nombre_variable variable);
int nombreToInt(t_nombre_variable variable);
bool esVariableDeclarada(t_elemento_stack* item, t_nombre_variable* variable);
int tipo_variable(t_nombre_variable variable, t_elemento_stack* head);
bool existeLabel(t_nombre_etiqueta label);
t_puntero_instruccion obtenerPosicionLabel(t_nombre_etiqueta label);
void validarOverflow(t_puntero direccion);
void enviarDireccionAMemoria(t_puntero direccion);
t_puntero obtener_posicion_de(t_nombre_variable variable);
t_puntero definir_variable(t_nombre_variable variable);
t_valor_variable dereferenciar_variable(t_puntero direccion_variable);
void asignar(t_puntero direccion_variable, t_valor_variable valor);
void ir_al_label(t_nombre_etiqueta label);
void finalizar();

t_valor_variable obtener_valor_compartida(t_nombre_compartida nombreVariableCompartida);
t_valor_variable asignar_valor_compartida(t_nombre_compartida nombreVariableCompartida, t_valor_variable valorCompartida);
void llamar_sin_retorno(t_nombre_etiqueta etiqueta);
void llamar_con_retorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar);
void retornar(t_valor_variable unaVariable);
void wait(t_nombre_semaforo identificador_semaforo);
void primitiva_signal(t_nombre_semaforo identificador_semaforo);


#endif /* PRIMITIVAS_H_ */
