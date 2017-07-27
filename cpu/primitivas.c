#include "primitivas.h"

bool esParametro(t_nombre_variable variable) {
//El enunciado aclara por que hacemos esto en el apendice de ANSISOP
	return (variable >= '0' && variable <= '9');
}

int32_t nombreToInt(t_nombre_variable variable){
	return variable - '0';
}

bool esVariableDeclarada(t_elemento_stack* item, t_nombre_variable* variable) {
	return dictionary_has_key(item->identificadores, variable);
}

int32_t tipo_variable(t_nombre_variable variable, t_elemento_stack* head) {
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

void enviarDireccionAMemoria(t_puntero direccion) {
	int32_t pagina = (int32_t)(direccion/tamanioPaginas) + cantidadPagCodigo; //Agrego el desplazamiento por las paginas ya ocupadas por el codigo
	int32_t offset = direccion % tamanioPaginas;
	int32_t size = sizeof(int32_t);
	int32_t pid = pcbNuevo->PID;

	enviarSolicitudBytes(pid, pagina, offset, size);
}

void enviar_direccion_y_valor_a_Memoria(t_puntero direccion, t_valor_variable valor) {
	int32_t pagina = (int32_t)(direccion/tamanioPaginas) + cantidadPagCodigo; //Agrego el desplazamiento por las paginas ya ocupadas por el codigo
	int32_t offset = direccion % tamanioPaginas;
	int32_t size = sizeof(int32_t);
	int32_t pid = pcbNuevo->PID;

	enviarAlmacenarBytes(pid, pagina, offset, size, valor);
}

char* convertirFlags(t_banderas flags){
	char* permisos = string_new();
	if(flags.creacion)
	string_append(&permisos,"c");
	if(flags.escritura)
	string_append(&permisos,"w");
	if(flags.lectura)
	string_append(&permisos,"r");
	return permisos;
}

/******************************* PRIMITIVAS ******************************/

t_puntero obtener_posicion_de(t_nombre_variable variable) {

	if(!ejecucionInterrumpida){

		log_debug(debugLog, ANSI_COLOR_YELLOW "OBTENER_POSICION_DE");
		log_debug(debugLog, ANSI_COLOR_BLUE "PID:  |%d|", pcbNuevo->PID);
		log_debug(debugLog, "La primitiva recibio la VARIABLE: |%c| ", variable);

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
			finalizar_proceso(false,false);
		}

		log_debug(debugLog, "La pos_relativa es pagina: |%d|, offset: |%d| ", posicionRelativa->nroPagina, posicionRelativa->offset);
		log_debug(debugLog, "La pos_absoluta: |%d|", posicionAbsoluta);
		loggearFinDePrimitiva("obtener_posicion_de");

		free(cadena);
		return posicionAbsoluta;
	}else
	{
		return 0;
	}
}

t_puntero definir_variable(t_nombre_variable variable) {

if(!ejecucionInterrumpida){
		log_debug(debugLog, ANSI_COLOR_YELLOW "DEFINIR_VARIABLE");
		log_debug(debugLog, ANSI_COLOR_BLUE "PID:  |%d|", pcbNuevo->PID);
		log_debug(debugLog, "La primitiva recibio la VARIABLE: |%c| ", variable);
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
	else
	{
		return 0;
	}

}

t_valor_variable dereferenciar_variable(t_puntero direccion_variable)
{
	if(!ejecucionInterrumpida){

	log_debug(debugLog, ANSI_COLOR_YELLOW "DEREFERENCIAR_VARIABLE");
	log_debug(debugLog, ANSI_COLOR_BLUE "PID:  |%d|", pcbNuevo->PID);
	log_debug(debugLog, "La primitiva recibio la direccion: |%d| ", direccion_variable);

	t_valor_variable valor = 0;

		//manda codigo de accion
		char* accion = (char*)solicitarBytesAccion;
		send(memoria, accion, sizeof(accion), 0);

		//manda pedidoa memoria
		enviarDireccionAMemoria(direccion_variable);

			//recibe valor de memoria
			char* bufferValor = malloc(sizeof(int32_t));
			int32_t valorRecibido = recv(memoria, bufferValor, sizeof(int32_t), 0);
			if (valorRecibido <= 0)
			{
				perror("recv devolvio un numero menor que cero");
				exit(1);
			}

			valor = char4ToInt(bufferValor);

			log_debug(debugLog, "La primitiva recibio el |%d| de la posicion |%d|.", valor, direccion_variable);
			loggearFinDePrimitiva("dereferenciar_variable");

			free(bufferValor);

			return valor;
	}else{
		return 0;
	}

}

void asignar(t_puntero direccion_variable, t_valor_variable valor)
{
	if(!ejecucionInterrumpida){

	log_debug(debugLog, ANSI_COLOR_YELLOW "ASIGNAR");
	log_debug(debugLog, ANSI_COLOR_BLUE "PID:  |%d|", pcbNuevo->PID);
	log_debug(debugLog, "La primitiva recibio la direccion: |%d|, con el valor: |%d| ", direccion_variable, valor);

	//Manda pedido a memoria
	enviar_direccion_y_valor_a_Memoria(direccion_variable, valor);

	loggearFinDePrimitiva("asignar");

	}
}

void ir_al_label(t_nombre_etiqueta label)
{
	if(!ejecucionInterrumpida){

	log_debug(debugLog, ANSI_COLOR_YELLOW "IR_AL_LABEL");
	log_debug(debugLog, ANSI_COLOR_BLUE "PID:  |%d|", pcbNuevo->PID);
	log_debug(debugLog, "La primitiva recibio el label: |%d| ", label);

	t_puntero_instruccion posPrimeraInstruccionUtil = -1;

	posPrimeraInstruccionUtil = metadata_buscar_etiqueta(label, pcbNuevo->indiceEtiquetas, pcbNuevo->etiquetasSize);

	log_debug(debugLog, "Se actualiza el PC del PCB a: |%d| ", posPrimeraInstruccionUtil);
	actualizarPC(pcbNuevo, posPrimeraInstruccionUtil);

	loggearFinDePrimitiva("ir_al_label");

	return;
	}
}

void finalizar()
{
	if(!ejecucionInterrumpida){
	log_debug(debugLog, ANSI_COLOR_YELLOW "FINALIZAR");
	log_debug(debugLog, ANSI_COLOR_BLUE "PID:  |%d|", pcbNuevo->PID);

	t_elemento_stack* head = stack_pop(stack);

	//posRetorno coincide con el PC. Es a donde tiene que volver
	t_puntero_instruccion retorno = head->posRetorno;

	stack_elemento_destruir(head);

	int32_t elementos = stack_tamanio(stack);

	//actualizarPC(pcbNuevo, retorno);

	loggearFinDePrimitiva("finalizar");
	}

}

t_valor_variable obtener_valor_compartida(t_nombre_compartida nombreVariableCompartida)
{
	if(!ejecucionInterrumpida){
	log_debug(debugLog, ANSI_COLOR_YELLOW "OBTENER_VALOR_COMPARTIDA");
	log_debug(debugLog, ANSI_COLOR_BLUE "PID:  |%d|", pcbNuevo->PID);
	log_debug(debugLog, "Se pide a kernel el valor de la variable: |%s| ", nombreVariableCompartida);

	t_valor_variable valorCompartida;
	int32_t codigoAccion = accionObtenerValorCompartida;

	int32_t tamanioNombreCom = strlen(nombreVariableCompartida) + 1;

	void* buffer = malloc(sizeof(int32_t)*2 + tamanioNombreCom);
	memcpy(buffer, &codigoAccion, sizeof(codigoAccion));
	memcpy(buffer + sizeof(codigoAccion), &tamanioNombreCom, sizeof(tamanioNombreCom));
	memcpy(buffer + sizeof(codigoAccion) + sizeof(tamanioNombreCom), nombreVariableCompartida, tamanioNombreCom);

	send(kernel, buffer, sizeof(int32_t)*2 + tamanioNombreCom, 0);

	recv(kernel, &valorCompartida, sizeof(int32_t), 0);

	log_debug(debugLog, "El valor es: |%d| ", valorCompartida);
	loggearFinDePrimitiva("obtener_valor_compartida");

	return valorCompartida;
	}else
	{
		return 0;
	}

}

t_valor_variable asignar_valor_compartida(t_nombre_compartida nombreVariableCompartida, t_valor_variable valorCompartida)
{
	if(!ejecucionInterrumpida){

		log_debug(debugLog, ANSI_COLOR_YELLOW "ASIGNAR_VALOR_COMPARTIDA");
	log_debug(debugLog, ANSI_COLOR_BLUE "PID:  |%d|", pcbNuevo->PID);
	log_debug(debugLog, "Se pide a kernel asignar: |%d| a la variable: |%s| ", valorCompartida, nombreVariableCompartida);

	t_valor_variable valorAsignado;

	int32_t codigoAccion = accionAsignarValorCompartida;

	int32_t tamanioNombreCom = strlen(nombreVariableCompartida) + 1;

	void* buffer = malloc(sizeof(int32_t)*3 + tamanioNombreCom);
	memcpy(buffer, &codigoAccion, sizeof(codigoAccion));
	memcpy(buffer + sizeof(codigoAccion), &tamanioNombreCom, sizeof(tamanioNombreCom));
	memcpy(buffer + sizeof(codigoAccion) + sizeof(tamanioNombreCom), nombreVariableCompartida, tamanioNombreCom);
	memcpy(buffer + sizeof(codigoAccion) + sizeof(tamanioNombreCom) + tamanioNombreCom, &valorCompartida, sizeof(valorCompartida));

	send(kernel, buffer, sizeof(int32_t)*3 + tamanioNombreCom, 0);

	recv(kernel, &valorAsignado,sizeof(int32_t), 0);

	loggearFinDePrimitiva("asignar_valor_compartida");

	if(valorAsignado == valorCompartida){
		log_debug(debugLog, "Se asigno el valor: |%c| ", valorAsignado);
		return valorAsignado;
	}
	else
	{
		//TODO:ERROR!
		return 0;
	}

	return valorAsignado;
	}else
	{
		return 0;

	}
}

void llamar_sin_retorno(t_nombre_etiqueta etiqueta)
{
	if(!ejecucionInterrumpida){

	log_debug(debugLog, ANSI_COLOR_YELLOW "LLAMAR_SIN_RETORNO");
	log_debug(debugLog, ANSI_COLOR_BLUE "PID:  |%d|", pcbNuevo->PID);
	log_debug(debugLog, "Se llama a la funcion |%c| ", etiqueta);

	t_puntero_instruccion posicionFuncion = metadata_buscar_etiqueta(etiqueta, pcbNuevo->indiceEtiquetas, pcbNuevo->etiquetasSize);
	t_elemento_stack* newHead = stack_elemento_crear();

	//dondeRetornar
	int32_t posicion = pcbNuevo->contadorPrograma;
	newHead->posRetorno = ++posicion;

	log_debug(debugLog, "Se retorna a la posicion |%d| ", posicion);

	// Si el stack tiene pos 0, size=1, si tiene 0 y 1, size=2,... Da la posicion del lugar nuevo.
	newHead->pos = stack_tamanio(stack);

	//newHead->valorDeRetorno es el parser quien en retornar le pasa en que variable guardar el resultado.
	stack_push(stack, newHead);

	actualizarPC(pcbNuevo, posicionFuncion);

	log_debug(debugLog, "Se actualiza el PC: |%d| ", posicionFuncion);

	loggearFinDePrimitiva("llamar_sin_retorno");

	}

}

void llamar_con_retorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar)
{
	if(!ejecucionInterrumpida){

	log_debug(debugLog, ANSI_COLOR_YELLOW "LLAMAR_CON_RETORNO");
	log_debug(debugLog, ANSI_COLOR_BLUE "PID:  |%d|", pcbNuevo->PID);
	log_debug(debugLog, "Se llama a la funcion: |%s| y se retornara luego a: |%d| ", etiqueta, donde_retornar);

	t_puntero_instruccion posicionFuncion = metadata_buscar_etiqueta(etiqueta, pcbNuevo->indiceEtiquetas, pcbNuevo->etiquetasSize);

	t_elemento_stack* newHead = stack_elemento_crear();

	newHead->valRetorno.nroPagina = (int32_t)(donde_retornar/tamanioPaginas) + cantidadPagCodigo;
	newHead->valRetorno.offset = donde_retornar % tamanioPaginas;
	newHead->valRetorno.size = sizeof(int32_t);

	//dondeRetornar
	newHead->posRetorno = pcbNuevo->contadorPrograma;
	// Si el stack tiene pos 0, size=1, si tiene 0 y 1, size=2,... Da la posicion del lugar nuevo.
	newHead->pos = stack_tamanio(stack);
	//newHead->valorDeRetorno es el parser quien en retornar le pasa en que variable guardar el resultado.
	stack_push(stack, newHead);

	actualizarPC(pcbNuevo, posicionFuncion);

	log_debug(debugLog, "Se actualiza el PC: |%d| ", posicionFuncion);

	loggearFinDePrimitiva("llamar_con_retorno");

	return;
	}
}

void retornar(t_valor_variable unaVariable)
{
	if(!ejecucionInterrumpida){

	log_debug(debugLog, ANSI_COLOR_YELLOW "RETORNAR");
	log_debug(debugLog, ANSI_COLOR_BLUE "PID:  |%d|", pcbNuevo->PID);
	log_debug(debugLog, "Se retorna: |%d|.", unaVariable);

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

	log_debug(debugLog, "Se actualiza el PC: |%d| ", retorno);


	loggearFinDePrimitiva("retornar");

	return;
	}
}

void wait(t_nombre_semaforo identificador_semaforo)
{
	if(!ejecucionInterrumpida){

	log_debug(debugLog, ANSI_COLOR_YELLOW "WAIT");
	log_debug(debugLog, ANSI_COLOR_BLUE "PID:  |%d|", pcbNuevo->PID);
	log_debug(debugLog, "El semaforo es: |%s|.", identificador_semaforo);

	char* nombreSemaforo = identificador_semaforo;
	int32_t codigoAccion = accionWait;
	int32_t tamanioNombreSem = strlen(nombreSemaforo) + 1;

	void* buffer = malloc(sizeof(int32_t)*2 + tamanioNombreSem);
	memcpy(buffer, &codigoAccion, sizeof(codigoAccion));
	memcpy(buffer + sizeof(codigoAccion), &tamanioNombreSem, sizeof(tamanioNombreSem));
	memcpy(buffer + sizeof(codigoAccion) + sizeof(tamanioNombreSem), nombreSemaforo, tamanioNombreSem);

	send(kernel, buffer, sizeof(int32_t)*2 + tamanioNombreSem, 0);

	loggearFinDePrimitiva("wait");
	}
}

void primitiva_signal(t_nombre_semaforo identificador_semaforo)
{
	if(!ejecucionInterrumpida){
	log_debug(debugLog, ANSI_COLOR_YELLOW "SIGNAL");
	log_debug(debugLog, ANSI_COLOR_BLUE "PID:  |%d|", pcbNuevo->PID);
	log_debug(debugLog, "El semaforo es: |%c|.", identificador_semaforo);

	char* nombreSemaforo = identificador_semaforo;
	int32_t codigoAccion = accionSignal;
	enviarTamanioYString(codigoAccion, kernel, nombreSemaforo);
	loggearFinDePrimitiva("signal");
	}
}

t_puntero reservar(t_valor_variable espacio)
{
	if(!ejecucionInterrumpida){

	log_debug(debugLog, ANSI_COLOR_YELLOW "RESERVAR");
	log_debug(debugLog, ANSI_COLOR_BLUE "PID:  |%d|", pcbNuevo->PID);
	log_debug(debugLog, "La primitiva recibio para reservar: |%d| de espacio.", espacio);
	int32_t codigoAccion = accionReservarHeap;
	int32_t pid = pcbNuevo->PID;
	int32_t espacioParaAlocar = espacio;

	void* buffer = malloc(sizeof(int32_t)*3);
	memcpy(buffer, &codigoAccion, sizeof(codigoAccion));
	memcpy(buffer + sizeof(codigoAccion), &pid, sizeof(pid));
	memcpy(buffer + sizeof(codigoAccion) + sizeof(int32_t), &espacioParaAlocar, sizeof(pid));

	send(kernel, buffer, sizeof(int32_t)*3, 0);

	int32_t puntero;
	recv(kernel, &puntero, sizeof(int32_t), 0);

	if(puntero < 0)
	{
		lanzar_excepcion(pcbNuevo, ERROR_SOLICITUD_HEAP);
		ejecucionInterrumpida = true;
	}

	log_debug(debugLog, "La primitiva recibio el puntero: |%d| .", puntero);
	loggearFinDePrimitiva("reservar");

	return puntero;
	}else
	{
		return 0;
	}
}

void liberar(t_puntero puntero)
{
	if(!ejecucionInterrumpida){

	log_debug(debugLog, ANSI_COLOR_YELLOW "LIBERAR");
	log_debug(debugLog, ANSI_COLOR_BLUE "PID:  |%d|", pcbNuevo->PID);
	log_debug(debugLog, "La primitiva recibio el puntero: |%d| para liberar.", puntero);

	int32_t codigoAccion = accionLiberarHeap;
	int32_t pid = pcbNuevo->PID;
	int32_t punteroALiberar = puntero;
	int32_t cantPagCodigo = pcbNuevo->cantidadPaginas;

	void* buffer = malloc(sizeof(int32_t)*4);
	memcpy(buffer, &codigoAccion, sizeof(codigoAccion));
	memcpy(buffer + sizeof(codigoAccion), &pid, sizeof(pid));
	memcpy(buffer + sizeof(codigoAccion) + sizeof(int32_t), &punteroALiberar, sizeof(punteroALiberar));
	memcpy(buffer + sizeof(codigoAccion) + sizeof(int32_t) + sizeof(punteroALiberar), &cantPagCodigo, sizeof(cantPagCodigo));

	send(kernel, buffer, sizeof(int32_t)*4, 0);

	int32_t respuesta = 0;
	recv(kernel, &respuesta, sizeof(int32_t), 0);
	if(respuesta == 1)
	{
		loggearFinDePrimitiva("liberar");
	}
	else
	{
		log_error(debugLog, "ERROR AL LIBERAR");
	}
}
}

t_descriptor_archivo abrir(t_direccion_archivo direccion, t_banderas flags)
{
	if(!ejecucionInterrumpida){


	log_debug(debugLog, ANSI_COLOR_YELLOW "ABRIR");
	log_debug(debugLog, "La primitiva recibio la direccion: |%s|", direccion);

	//Envio comando al kernel
	int32_t codigoAccion = accionAbrirArchivo;
	int32_t pid = pcbNuevo->PID;
	int32_t tamanioPath = string_length(direccion) + 1;
	char* permisos = convertirFlags(flags);
	int32_t tamanioPermisos = string_length(permisos) + 1;
	int32_t tamanioBuffer = sizeof(int32_t)*4 + tamanioPath + tamanioPermisos;
	void* buffer = malloc(tamanioBuffer);
	int32_t offset = 0;
	memcpy(buffer, &codigoAccion, sizeof(int32_t));
	offset += sizeof(int32_t);
	memcpy(buffer + offset, &pid, sizeof(int32_t));
	offset += sizeof(int32_t);
	memcpy(buffer + offset, &tamanioPath, sizeof(int32_t));
	offset += sizeof(int32_t);
	memcpy(buffer + offset, &tamanioPermisos, sizeof(int32_t));
	offset += sizeof(int32_t);
	memcpy(buffer + offset, direccion, tamanioPath);
	offset += tamanioPath;
	memcpy(buffer + offset, permisos, tamanioPermisos);
	send(kernel, buffer, tamanioBuffer, 0);

	int32_t fd;
	recv(kernel, &fd, sizeof(int32_t), 0);

	if(fd<0)
	{
		lanzar_excepcion(pcbNuevo, ERROR_ACCESO_ARCHIVO);
		ejecucionInterrumpida = true;
	}

	return fd;

	loggearFinDePrimitiva("abrir");
	}else
	{
		return 0;
	}
}

void borrar(t_descriptor_archivo direccion)
{
	if(!ejecucionInterrumpida){


	log_debug(debugLog, ANSI_COLOR_YELLOW "BORRAR");
	log_debug(debugLog, "La primitiva recibio el descriptor: |%d|", direccion);

	//Envio comando al kernel
	int32_t codigoAccion = accionBorrarArchivo;
	int32_t fd= (int32_t)direccion;
	int32_t pid = (int32_t) pcbNuevo->PID;
	int32_t tamanioBuffer = sizeof(int32_t)*3;
	void* buffer = malloc(tamanioBuffer);
	int32_t offset = 0;
	memcpy(buffer, &codigoAccion, sizeof(int32_t));
	offset += sizeof(int32_t);
	memcpy(buffer + offset, &fd, sizeof(int32_t));
	offset += sizeof(int32_t);
	memcpy(buffer + offset, &pid, sizeof(int32_t));
	send(kernel, buffer, tamanioBuffer, 0);

	loggearFinDePrimitiva("borrar");
	}
}

void cerrar(t_descriptor_archivo descriptor_archivo)
{
	if(!ejecucionInterrumpida){
	log_debug(debugLog, ANSI_COLOR_YELLOW "CERRAR");
	log_debug(debugLog, "La primitiva recibio el descriptor: |%d|", descriptor_archivo);

	//Envio comando al kernel
	int32_t codigoAccion = accionCerrarArchivo;
	int32_t fd= (int32_t)descriptor_archivo;
	int32_t pid = (int32_t) pcbNuevo->PID;
	int32_t tamanioBuffer = sizeof(int32_t)*3;
	void* buffer = malloc(tamanioBuffer);
	int32_t offset = 0;
	memcpy(buffer, &codigoAccion, sizeof(int32_t));
	offset += sizeof(int32_t);
	memcpy(buffer + offset, &fd, sizeof(int32_t));
	offset += sizeof(int32_t);
	memcpy(buffer + offset, &pid, sizeof(int32_t));
	send(kernel, buffer, tamanioBuffer, 0);

	loggearFinDePrimitiva("cerrar");
	}
}

void mover_cursor(t_descriptor_archivo descriptor_archivo, t_valor_variable posicion)
{
	if(!ejecucionInterrumpida){
	log_debug(debugLog, ANSI_COLOR_YELLOW "MOVER CURSOR");
	log_debug(debugLog, "La primitiva recibio el descriptor: |%d|, y la posicion |%d|", descriptor_archivo, posicion);

	//Envio comando al kernel
	int32_t codigoAccion = accionMoverCursor;
	int32_t fd= (int32_t)descriptor_archivo;
	int32_t pid = (int32_t) pcbNuevo->PID;
	int32_t tamanioBuffer = sizeof(int32_t)*4;
	void* buffer = malloc(tamanioBuffer);
	int32_t offset = 0;
	memcpy(buffer, &codigoAccion, sizeof(int32_t));
	offset += sizeof(int32_t);
	memcpy(buffer + offset, &fd, sizeof(int32_t));
	offset += sizeof(int32_t);
	memcpy(buffer + offset, &pid, sizeof(int32_t));
	offset += sizeof(int32_t);
	memcpy(buffer + offset, &posicion, sizeof(int32_t));
	send(kernel, buffer, tamanioBuffer, 0);

	loggearFinDePrimitiva("mover_cursor");
	}
}

void escribir(t_descriptor_archivo descriptor_archivo, void* informacion, t_valor_variable tamanio)
{
	if(!ejecucionInterrumpida){
	log_debug(debugLog, ANSI_COLOR_YELLOW "ESCRIBIR");
	log_debug(debugLog, "La primitiva recibio el descriptor: |%d|, y un tamanio |%d|", descriptor_archivo, tamanio);

	//Envio comando al kernel
	int32_t codigoAccion = accionEscribir;
	int32_t fd = (int32_t) descriptor_archivo;
	int32_t pid = (int32_t) pcbNuevo->PID;
	int32_t tamanioBuffer = sizeof(int32_t)*4 + tamanio;
	void* buffer = malloc(tamanioBuffer);
	int32_t offset = 0;
	memcpy(buffer, &codigoAccion, sizeof(int32_t));
	offset += sizeof(int32_t);
	memcpy(buffer + offset, &fd, sizeof(int32_t));
	offset += sizeof(int32_t);
	memcpy(buffer + offset, &pid, sizeof(int32_t));
	offset += sizeof(int32_t);
	memcpy(buffer + offset, &tamanio, sizeof(int32_t));
	offset += sizeof(int32_t);
	memcpy(buffer + offset, informacion, tamanio);
	send(kernel, buffer, tamanioBuffer, 0);


	int res;
	recv(kernel, &res, sizeof(int32_t),0);
	if(res < 0)
	{
		lanzar_excepcion(pcbNuevo, ERROR_ESCRITURA);
		ejecucionInterrumpida = true;
	}
			}
		loggearFinDePrimitiva("escribir");
	}
}

void leer(t_descriptor_archivo descriptor_archivo, t_puntero informacion, t_valor_variable tamanio)
{
	if(!ejecucionInterrumpida){
	log_debug(debugLog, ANSI_COLOR_YELLOW "LEER");
	log_debug(debugLog, "La primitiva recibio el descriptor |%d|, un tamanio |%d|, y un puntero |%d|", descriptor_archivo, tamanio, informacion);

	//Envio comando al kernel
	int32_t codigoAccion = accionObtenerDatosArchivo;
	int32_t fd= (int32_t)descriptor_archivo;
	int32_t pid = (int32_t) pcbNuevo->PID;
	int32_t tamanioBuffer = sizeof(int32_t)*4;
	void* buffer = malloc(tamanioBuffer);
	int32_t offset = 0;
	memcpy(buffer, &codigoAccion, sizeof(int32_t));
	offset += sizeof(int32_t);
	memcpy(buffer + offset, &fd, sizeof(int32_t));
	offset += sizeof(int32_t);
	memcpy(buffer + offset, &pid, sizeof(int32_t));
	offset += sizeof(int32_t);
	memcpy(buffer + offset, &tamanio, sizeof(int32_t));
	send(kernel, buffer, tamanioBuffer, 0);

	int res;
	recv(kernel, &res, sizeof(int32_t),0);
	if(res != 1)
	{
		char *recibido = malloc(tamanio);
		recv(kernel, recibido, tamanio, 0);
		//Envio datos a memoria
		int32_t i=0;
		while(i<tamanio){
			enviar_direccion_y_valor_a_Memoria(informacion+i, recibido[i]);
			log_debug(debugLog, "Enviando a memoria. Pos: |%d| Valor: |%c|", informacion+i, recibido[i]);
			i++;
		}
	}else
	{
		lanzar_excepcion(pcbNuevo, ERROR_PERMISOS);
		ejecucionInterrumpida = true;
	}



	loggearFinDePrimitiva("leer");
	}
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
	funcionesKernel.AnSISOP_signal = &primitiva_signal;
	funcionesKernel.AnSISOP_wait = &wait;



}


