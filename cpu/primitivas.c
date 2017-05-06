#include "primitivas.h"

t_puntero obtener_posicion_de(t_nombre_variable variable) {

	//Implementarla!

	return 0;
}

t_puntero definir_variable(t_nombre_variable variable) {

	//Implementarla!

  return 0;
}

//Esta asigna todas las implementaciones nuestras al enumerador de funciones del parser de SO
void inicializarPrimitivas() {
	funciones.AnSISOP_definirVariable = &definir_variable;
	funciones.AnSISOP_obtenerPosicionVariable = &obtener_posicion_de;
}


