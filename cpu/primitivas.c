#include "primitivas.h"

t_puntero obtener_posicion_de(t_nombre_variable variable) {

	//Implementarla!

	return 0;
}

t_puntero definir_variable(t_nombre_variable variable) {

	//Implementarla!

  return 0;
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


