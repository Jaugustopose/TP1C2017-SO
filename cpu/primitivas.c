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
void enviar_direccion_y_valor_a_Memoria(t_puntero direccion, t_valor_variable valor) {
	int pagina = (int)(direccion/tamanioPaginas) + cantidadPagCodigo; //Agrego el desplazamiento por las paginas ya ocupadas por el codigo
	int offset = direccion % tamanioPaginas;
	int size = sizeof(int);
	int pid = pcbNuevo->PID;

	enviarAlmacenarBytes(pid, pagina, offset, size, valor);
}

/******************************* PRIMITIVAS ******************************/

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
			posicionRelativa = (t_pedido*)list_get(head->argumentos, nombreToInt(variable));
			break;
		case NOEXISTE:
			posicionAbsoluta = -1;
			break;
	}

	if (posicionAbsoluta != (t_puntero)-1) {
			posicionAbsoluta = (posicionRelativa->nroPagina*tamanioPaginas) + posicionRelativa->offset;
	} else {
		finalizarProcesoVariableInvalida();
	}

	loggearFinDePrimitiva("obtener_posicion_de");

	free(cadena);
	return posicionAbsoluta;
}

t_puntero definir_variable(t_nombre_variable variable) {

	//t_pedido* direccion = stack_proximo_pedido(stack, tamanioPaginas);

	//ATENCION: ACA SE TOMAN LAS DIRECCIONES LOGICAS: CODIGO + STACK + HEAP
	t_pedido* direccion = stack_proximo_pedido(stack, tamanioPaginas, cantidadPagCodigo);
	t_elemento_stack* head = stack_head(stack);
	char* cadena = string_from_format("%c",variable);

	if(esParametro(variable))
	{
		list_add(head->argumentos,(void*)direccion);
	}else{
		//agrego el caracter a una cadena
	 dictionary_put(head->identificadores, cadena, (void*)direccion);
	}

	loggearFinDePrimitiva("definir_variable");

	free(cadena);
	//free(direccion);

	return (direccion->nroPagina*tamanioPaginas) + direccion->offset;
}

t_valor_variable dereferenciar_variable(t_puntero direccion_variable)
{
		t_valor_variable valor;

		//manda codigo de accion
		char* accion = (char*)solicitarBytesAccion;
		send(memoria, accion, sizeof(accion), 0);

		//manda pedidoa memoria
		enviarDireccionAMemoria(direccion_variable);

	//	if(!hayOverflow()){

			//recibe valor de memoria
			char* bufferValor = malloc(sizeof(int));
			int valorRecibido = recv(memoria, bufferValor, sizeof(int), 0);
			if (valorRecibido <= 0)
			{
				perror("recv devolvio un numero menor que cero");
				exit(1);
			}

			valor = char4ToInt(bufferValor);

			loggearFinDePrimitiva("dereferenciar_variable");

			free(bufferValor);

			return valor;

//		}
//		else{
//			return overflowException(overflow);
//		}
}

void asignar(t_puntero direccion_variable, t_valor_variable valor)
{
	//Manda pedido a memoria
	enviar_direccion_y_valor_a_Memoria(direccion_variable, valor);

	loggearFinDePrimitiva("asignar");

	return;
}

void ir_al_label(t_nombre_etiqueta label)
{
	t_puntero_instruccion posPrimeraInstruccionUtil = -1;

	if (existeLabel(label)) {
		posPrimeraInstruccionUtil = obtenerPosicionLabel(label);
	}
	else
	{
		//ERRROR!
		//devuelve posPrimeraInstruccionUtil = -1
	}

	actualizarPC(pcbNuevo, posPrimeraInstruccionUtil);

	loggearFinDePrimitiva("ir_al_label");

	return;
}

void finalizar()
{
	t_elemento_stack* head = stack_pop(stack);

	//posRetorno coincide con el PC. Es a donde tiene que volver
	t_puntero_instruccion retorno = head->posRetorno;

	stack_elemento_destruir(head);

	int elementos = stack_tamanio(stack);

	if(elementos <= 0)
	{

	}

	actualizarPC(pcbNuevo, retorno);

	loggearFinDePrimitiva("finalizar");


}

t_valor_variable obtener_valor_compartida(t_nombre_compartida nombreVariableCompartida)
{
	t_valor_variable valorCompartida;
	int codigoAccion = accionObtenerValorCompartida;

	enviarTamanioYString(codigoAccion, kernel, nombreVariableCompartida);

	recv(kernel, &valorCompartida, sizeof(int), 0);

	loggearFinDePrimitiva("obtener_valor_compartida");

	return valorCompartida;

}

t_valor_variable asignar_valor_compartida(t_nombre_compartida nombreVariableCompartida, t_valor_variable valorCompartida)
{
	t_valor_variable valorAsignado;

	int codigoAccion = accionAsignarValorCompartida;

	enviarTamanioYString(codigoAccion, kernel, nombreVariableCompartida);

	//envio el valor
	char* valor = intToChar4(valorCompartida);
	send(kernel, valor, sizeof(int), 0);

	recv(kernel, &valorAsignado,sizeof(int), 0);

	if(valorAsignado == *valor){
		return valorAsignado;
	}
	else
	{
		//TODO:ERROR!
		return 0;
	}

	loggearFinDePrimitiva("asignar_valor_compartida");

	return valorAsignado;
}

void llamar_sin_retorno(t_nombre_etiqueta etiqueta)
{
	t_puntero_instruccion posicionFuncion =  obtenerPosicionLabel(etiqueta);
	t_elemento_stack* newHead = stack_elemento_crear();

	//dondeRetornar
	newHead->posRetorno = pcbNuevo->contadorPrograma;

	// Si el stack tiene pos 0, size=1, si tiene 0 y 1, size=2,... Da la posicion del lugar nuevo.
	newHead->pos = stack_tamanio(stack);

	//newHead->valorDeRetorno es el parser quien en retornar le pasa en que variable guardar el resultado.
	stack_push(stack, newHead);

	actualizarPC(pcbNuevo, posicionFuncion);

	loggearFinDePrimitiva("llamar_sin_retorno");

}

void llamar_con_retorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar)
{
	t_puntero_instruccion posicionFuncion =  obtenerPosicionLabel(etiqueta);

	t_elemento_stack* newHead = stack_elemento_crear();

	newHead->valRetorno.nroPagina = (int)(donde_retornar/tamanioPaginas) + cantidadPagCodigo;
	newHead->valRetorno.offset = donde_retornar % tamanioPaginas;
	newHead->valRetorno.size = sizeof(int);

	//dondeRetornar
	newHead->posRetorno = pcbNuevo->contadorPrograma;
	// Si el stack tiene pos 0, size=1, si tiene 0 y 1, size=2,... Da la posicion del lugar nuevo.
	newHead->pos = stack_tamanio(stack);
	//newHead->valorDeRetorno es el parser quien en retornar le pasa en que variable guardar el resultado.
	stack_push(stack, newHead);

	actualizarPC(pcbNuevo, posicionFuncion);

	loggearFinDePrimitiva("llamar_con_retorno");

	return;
}

void retornar(t_valor_variable unaVariable)
{
	t_elemento_stack* head = stack_pop(stack);

	//posRetorno coincide con el PC. Es a donde tiene que volver
	t_puntero_instruccion retorno = head->posRetorno;

	 //Manda codigo de accion
     char* accion = (char*)almacenarBytesAccion;
     send(memoria, accion, sizeof(accion), 0);

	//Manda pedido a memoria
     enviarSolicitudBytes(pcbNuevo->PID,
    		 	 	 	  head->valRetorno.nroPagina,
						  head->valRetorno.offset,
						  head->valRetorno.size);


	// Libero ese nivel del stack, porque termino de ejecutarse la funcion que lo creo y ya no es necesario
	stack_elemento_destruir(head);

	actualizarPC(pcbNuevo, retorno);

	loggearFinDePrimitiva("retornar");

	return;
}

void wait(t_nombre_semaforo identificador_semaforo)
{
    int codigoAccion = accionWait;
    enviarTamanioYString(codigoAccion, kernel, identificador_semaforo);
	loggearFinDePrimitiva("wait");
}

void signal(t_nombre_semaforo identificador_semaforo)
{
	int codigoAccion = accionSignal;
	enviarTamanioYString(codigoAccion, kernel, identificador_semaforo);
	loggearFinDePrimitiva("signal");
}

t_puntero reservar(t_valor_variable espacio)
{
	int codigoAccion = accionReservarHeap;
	char* espacioSerial = intToChar4(espacio);
	send(kernel, espacioSerial, sizeof(int), 0);

	//TODO:recv del puntero

	loggearFinDePrimitiva("reservar");
}

void liberar(t_puntero puntero)
{
	int codigoAccion = accionLiberarHeap;
	char* punteroSerial = intToChar4(puntero);
	send(kernel, punteroSerial, sizeof(int), 0);

	loggearFinDePrimitiva("liberar");
}

t_descriptor_archivo abrir(t_direccion_archivo direccion, t_banderas flags)
{
	loggearFinDePrimitiva("abrir");
}

void borrar(t_descriptor_archivo direccion)
{
	loggearFinDePrimitiva("borrar");
}

void cerrar(t_descriptor_archivo descriptor_archivo)
{
	loggearFinDePrimitiva("cerrar");
}

void mover_cursor(t_descriptor_archivo descriptor_archivo, t_valor_variable posicion)
{
	loggearFinDePrimitiva("mover_cursor");
}

void escribir(t_descriptor_archivo descriptor_archivo, void* informacion, t_valor_variable tamanio)
{
	loggearFinDePrimitiva("escribir");
}

void leer(t_descriptor_archivo descriptor_archivo, t_puntero informacion, t_valor_variable tamanio)
{
	loggearFinDePrimitiva("leer");
}

//Esta asigna todas las implementaciones nuestras al enumerador de funciones del parser de SO
void inicializarPrimitivas() {

	funciones.AnSISOP_definirVariable = &definir_variable;
	funciones.AnSISOP_obtenerPosicionVariable = &obtener_posicion_de;
	funciones.AnSISOP_dereferenciar = &dereferenciar_variable;
	funciones.AnSISOP_asignar = &asignar;
	funciones.AnSISOP_obtenerValorCompartida = &obtener_valor_compartida;
	funciones.AnSISOP_asignarValorCompartida = &asignar_valor_compartida;
	funciones.AnSISOP_irAlLabel = &ir_al_label;
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


