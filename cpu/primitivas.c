#include "primitivas.h"


bool esParametro(t_nombre_variable variable) {
//El enunciado aclara por que hacemos esto en el apendice de ANSISOP
	return (variable >= '0' && variable <= '9');
}

int nombreToInt(t_nombre_variable variable){
	return variable - '0';
}

bool esVariableDeclarada(t_elemento_stack* item, t_nombre_variable* variable) {
	return dictionary_has_key(item->identificadores, variable);
}

int tipo_variable(t_nombre_variable variable, t_elemento_stack* head) {
	char* cadena = string_from_format("%c",variable);

	if (esVariableDeclarada(head, cadena)) {
			free(cadena);
		return DECLARADA;
	} else {
		if (esParametro(variable)) {
				free(cadena);
			return PARAMETRO;
		}
	}
		free(cadena);
	return NOEXISTE;
}

bool existeLabel(t_nombre_etiqueta label) {
	return dictionary_has_key(pcbNuevo->indiceEtiquetas, label);
}

t_puntero_instruccion obtenerPosicionLabel(t_nombre_etiqueta label){
	return *(t_puntero_instruccion*)dictionary_get(pcbNuevo->indiceEtiquetas, label);
}

void validarOverflow(t_puntero direccion) {

	//Agrego el desplazamiento por las paginas ya ocupadas por el codigo
	int pagina = (int)(direccion/tamanioPaginas) + cantidadPagCodigo;
	int offset = direccion % tamanioPaginas;
	int size = sizeof(int);
	int pid = pcbNuevo->PID;

	enviarSolicitudBytes(pid,pagina,offset,size);
}


void enviarDireccionAMemoria(t_puntero direccion) {
	int pagina = (int)(direccion/tamanioPaginas) + cantidadPagCodigo; //Agrego el desplazamiento por las paginas ya ocupadas por el codigo
	int offset = direccion % tamanioPaginas;
	int size = sizeof(int);
	int pid = pcbNuevo->PID;

	enviarSolicitudBytes(pid, pagina, offset, size);
}

/******************************* PRIMIIVAS ******************************/
t_puntero obtener_posicion_de(t_nombre_variable variable) {

	t_puntero posicionAbsoluta = 0;
	t_pedido* posicionRelativa;
	char* cadena = string_from_format("%c",variable);
	t_elemento_stack* head = stack_head(stack);

	switch (tipo_variable(variable, head)) {

		case DECLARADA:
			posicionRelativa = (t_pedido*)dictionary_get(head->identificadores,cadena);
			break;
		case PARAMETRO:
			posicionRelativa = (t_pedido*)list_get(head->argumentos,nombreToInt(variable));
			break;
		case NOEXISTE:
			posicionAbsoluta = -1;
			break;
	}

	if (posicionAbsoluta != (t_puntero)-1) {
			posicionAbsoluta = (posicionRelativa->nroPagina*tamanioPaginas) + posicionRelativa->offset;
	} else {
		finalizarProcesoVariableInvalida();
		goto fin;
	}


	free(cadena);
	return posicionAbsoluta;

	fin: return 0;
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
	if(salteaCircuitoConGoTo){goto fin;}

		t_valor_variable valor;
		char* accion = (char*)almacenarBytesAccion;
		send(memoria, accion, sizeof(accion), 0);

		//validarOverflow(direccion_variable);
		enviarDireccionAMemoria(direccion_variable);

		if(!hayOverflow()){

			//TODO:Encapsular esta porqueria
			char* buffer = malloc(sizeof(int));
			char* valorRecibido = recv(memoria, buffer, sizeof(int), MSG_WAITALL);
			if (valorRecibido <= 0)
			{
				perror("recv devolvio un numero menor que cero");
				exit(1);
			}

			free(buffer);

			valor = (int)valorRecibido;
			free(valorRecibido);

			return valor;
		}else{
			overflowException(overflow);
		}

		fin: return 0;
}

void asignar(t_puntero direccion_variable, t_valor_variable valor)
{
	if(salteaCircuitoConGoTo){goto fin;}

	char* accion = (char*)almacenarBytesAccion;
	send(memoria, accion, sizeof(accion), 0);

	//validarOverflow(direccion_variable);
	enviarDireccionAMemoria(direccion_variable);

	//VER OVERFLOW

	if(!hayOverflow()){

		//Revisar si es necesario serializar el int
		char* valorSerializado = (char*)valor;
		send(memoria, valorSerializado, sizeof(t_valor_variable), 0);

		free(valorSerializado);
	}else{
		overflowException(overflow);
	}
	return;
	fin: goToMagia();
}

void irAlLabel(t_nombre_etiqueta label)
{
	if(salteaCircuitoConGoTo){goto fin;}

		t_puntero_instruccion posPrimeraInstruccionUtil = -1;

		if (existeLabel(label)) {
			posPrimeraInstruccionUtil = obtenerPosicionLabel(label);
		}
		//Si no entra al if, devuelve posPrimeraInstruccionUtil = -1

		actualizarPC(pcbNuevo, posPrimeraInstruccionUtil);

		return;

		fin: goToMagia();
}


void finalizar()
{

}

t_valor_variable obtener_valor_compartida(t_nombre_compartida variable)
{

}

t_valor_variable asignar_valor_compartida(t_nombre_compartida variable, t_valor_variable valor)
{

}
void llamar_sin_retorno(t_nombre_etiqueta etiqueta)
{

}
void llamar_con_retorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar)
{

}

void retornar(t_valor_variable retorno)
{

}

void wait(t_nombre_semaforo identificador_semaforo)
{

}
void signal(t_nombre_semaforo identificador_semaforo)
{

}

t_puntero reservar(t_valor_variable espacio)
{

}

void liberar(t_puntero puntero)
{

}

t_descriptor_archivo abrir(t_direccion_archivo direccion, t_banderas flags)
{

}

void borrar(t_descriptor_archivo direccion)
{

}

void cerrar(t_descriptor_archivo descriptor_archivo)
{

}

void mover_cursor(t_descriptor_archivo descriptor_archivo, t_valor_variable posicion)
{

}

void escribir(t_descriptor_archivo descriptor_archivo, void* informacion, t_valor_variable tamanio)
{

}

void leer(t_descriptor_archivo descriptor_archivo, t_puntero informacion, t_valor_variable tamanio)
{

}

//Esta asigna todas las implementaciones nuestras al enumerador de funciones del parser de SO
void inicializarPrimitivas() {

	funciones.AnSISOP_definirVariable = &definir_variable;
	funciones.AnSISOP_obtenerPosicionVariable = &obtener_posicion_de;
	funciones.AnSISOP_dereferenciar = &desreferenciar_variable;
	funciones.AnSISOP_asignar = &asignar;
	funciones.AnSISOP_obtenerValorCompartida = &obtener_valor_compartida;
	funciones.AnSISOP_asignarValorCompartida = &asignar_valor_compartida;
	funciones.AnSISOP_irAlLabel = &irAlLabel;
	funciones.AnSISOP_llamarSinRetorno = &llamar_sin_retorno;
	funciones.AnSISOP_llamarConRetorno = &llamar_con_retorno;
	funciones.AnSISOP_finalizar = &finalizar;
	funciones.AnSISOP_retornar = &retornar;
	funcionesKernel.AnSISOP_abrir = &abrir;
	funcionesKernel.AnSISOP_borrar = &borrar;
	funcionesKernel.AnSISOP_cerrar = &cerrar;
	funcionesKernel.AnSISOP_escribir = &escribir;
	funcionesKernel.AnSISOP_leer = &leer;
	funcionesKernel.AnSISOP_liberar = &liberar;
	funcionesKernel.AnSISOP_moverCursor = &mover_cursor;
	funcionesKernel.AnSISOP_reservar = &reservar;
	funcionesKernel.AnSISOP_signal = &signal;
	funcionesKernel.AnSISOP_wait = &wait;



}


