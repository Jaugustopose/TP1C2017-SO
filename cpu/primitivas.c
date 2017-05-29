#include "primitivas.h"


bool esParametro(t_nombre_variable variable) {
//El enunciado aclara por que hacemos esto en el apendice de ANSISOP
	return (variable >= '0' && variable <= '9');
}


t_puntero obtener_posicion_de(t_nombre_variable variable) {

	//Implementarla!

	return 0;
}

t_puntero definir_variable(t_nombre_variable variable) {

	t_pedido* direccion = stack_proximo_pedido(stack, tamanioPaginas);
	t_elemento_stack* head = stack_head(stack);
	char* cadena = string_from_format("%c",variable);

	if(esParametro(variable))
	{
	 list_add(head->argumentos,(void*)direccion);
	}else{

		//agrego el caracter a una cadena
	 dictionary_put(head->identificadores, cadena, (void*) direccion);
	}

	free(cadena);

	return (direccion->nroPagina*tamanioPaginas) + direccion->offset;
}

t_valor_variable desreferenciar_variable(t_puntero direccion_variable)
{
	//Implementarla!

	  return 0;
}

void asignar(t_puntero direccion_variable, t_valor_variable valor)
{
	//Implementarla!
}

void finalizar(void)
{

}



//Esta asigna todas las implementaciones nuestras al enumerador de funciones del parser de SO
void inicializarPrimitivas() {
	funciones.AnSISOP_definirVariable = &definir_variable;
	funciones.AnSISOP_obtenerPosicionVariable = &obtener_posicion_de;
	funciones.AnSISOP_finalizar = &desreferenciar_variable;
	funciones.AnSISOP_asignar = &asignar;
	funciones.AnSISOP_dereferenciar = &finalizar;
}


