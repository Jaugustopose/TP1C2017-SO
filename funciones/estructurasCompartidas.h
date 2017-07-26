
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
	accionDesalojarProceso = 12,
	accionObtenerValorCompartida = 14,
	accionAsignarValorCompartida = 15,
	accionQuantumInterrumpido = 16,
	accionException = 17,
	accionError = 18,
	accionConsolaFinalizarNormalmente = 19,
	accionConsolaFinalizarErrorInstruccion = 20,
	accionImprimirTextoConsola = 21,
	accionWait = 13,
	accionSignal = 23,
	accionEscribir = 24, //puede ser en archivo o consola, depende el FD
	accionMoverCursor = 25,
	accionAbrirArchivo = 26,
	accionCerrarArchivo = 27,
	accionCrearArchivo = 28,
	accionBorrarArchivo = 29,
	accionObtenerDatosArchivo = 30,
	accionReservarHeap = 31,
	accionLiberarHeap = 32,
	accionEnviarStackSize = 33,
	liberarPaginaProcesoAccion = 34

};

enum accionConsolaKernel {
	listarProcesos = 1,
	operarSobreProceso = 2,
	obtenerTGArchivos = 3,
	modificarMultiprogramacion = 4,
	finalizarProcesoPorUsuario = 5,
	detenerPlanificacion = 6
};

enum tipoDeCliente {

	soyConsola = 1,
	soyCPU = 2
};

enum algoritmoElegido {

	SOY_RR = 1,
	SOY_FIFO = 2
};

typedef enum identidad {

	SOYCONSOLA = 0,
	SOYKERNEL = 1,
	SOYCPU = 2,
	SOYMEMORIA = 3,
	SOYFS = 4

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
	char* indiceEtiquetas;
	int etiquetasSize;
	int exitCode;
}t_PCB;

typedef struct {
	int32_t pidProceso;
	int32_t ConsolaDuenio;
	int32_t CpuDuenio;
	int32_t estado;
	t_PCB* PCB;
	int32_t rafagas;
	int32_t rafagasTotales;
	int32_t privilegiadas;
	int32_t cantidadPaginasHeap;
	int32_t cantidadAlocaciones;
	int32_t bytesAlocados;
	int32_t cantidadLiberaciones;
	int32_t bytesLiberados;
	bool sigusr1;
	bool abortado;
	char* semaforo;
	int32_t tamanioScript;
	char* codigoPrograma;
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
