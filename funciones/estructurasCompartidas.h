
#ifndef ESTRUCTURASCOMPARTIDAS_H_
#define ESTRUCTURASCOMPARTIDAS_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <parser/parser.h>
#include <parser/metadata_program.h>

enum tipoMensaje {
	inicializarProgramaAccion = 1,
	solicitarPaginasAccion = 2,
	almacenarBytesAccion = 3,
	solicitarBytesAccion = 4,
	finalizarProgramaAccion = 5,
	obtenerTamanioPaginas = 6,
	envioScript = 7,
	accionContinuarProceso = 8,
	accionObtenerPCB = 9,
	accionFinInstruccion = 10,
	accionFinProceso = 11,
	accionObtenerValorCompartida = 14,
	accionAsignarValorCompartida = 15,
	accionQuantumInterrumpido = 16,
	accionException = 17,
	accionError = 18,
	accionConsolaFinalizarNormalmente = 19,
	accionConsolaFinalizarErrorInstruccion = 20,
	accionImprimirTextoConsola = 21,
	accionWait = 22,
	accionSignal = 23,
	accionEscribir = 24, //puede ser en archivo o consola, depende el FD
	accionMoverCursor = 25,
	accionAbrirArchivo = 26,
	accionCrearArchivo = 27,
	accionBorrarArchivo = 28,
	accionObtenerDatosArchivo = 29,
	accionReservarHeap = 30,
	accionLiberarHeap = 31

};

enum tipoDeCliente {

	soyConsola = 1,
	soyCPU = 2
};

typedef enum identidad {

	SOYCONSOLA = 0,
	SOYKERNEL = 1,
	SOYCPU = 2,
	SOYMEMORIA = 3

};


typedef struct {
	int32_t nroPagina;
	int32_t offset;
	int32_t size;
}t_pedido;

typedef t_list t_stack;

typedef struct {
	int32_t pos;
	t_list* argumentos;
	t_dictionary* identificadores;
	t_puntero posRetorno;
	t_pedido valRetorno;
} t_elemento_stack;

typedef struct {
	int32_t PID;
	int32_t cantidadPaginas;
	int32_t contadorPrograma;
	t_list* indiceCodigo;
	t_stack* stackPointer;
	t_dictionary* indiceEtiquetas;
	int exitCode;
}t_PCB;

typedef struct {

	int32_t ConsolaDuenio;
	int32_t CpuDuenio;
	int32_t estado;
	t_PCB* PCB;
}t_proceso;

typedef struct{
	int32_t inicio;
	int32_t fin;
}t_sentencia;

typedef struct pedidoBytesMemoriaStruct {
	int32_t pid;
	int32_t	nroPagina;
	int32_t offset;
	int32_t tamanio;
} pedidoBytesMemoria_t;

typedef struct pedidoAlmacenarBytesMemoriaStruct {
	pedidoBytesMemoria_t pedidoBytes;
	void* buffer;
} pedidoAlmacenarBytesMemoria_t;

typedef struct pedidoSolicitudPaginasStruct {
	int32_t pid;
	int32_t cantidadPaginas;
} pedidoSolicitudPaginas_t;


#endif /* ESTRUCTURASCOMPARTIDAS_H_ */
