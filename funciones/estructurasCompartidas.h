
#ifndef ESTRUCTURASCOMPARTIDAS_H_
#define ESTRUCTURASCOMPARTIDAS_H_

#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <parser/parser.h>
#include <parser/metadata_program.h>
#include "logger.h""
#include <sys/socket.h>


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
	liberarPaginaProcesoAccion = 34,
	accionConsolaFinalizada = 35,
	accionConsolaDesconectada = 36

};

enum consolaKernelPedidos
{
	listadoProcesoCompleto = 40,
	totalRafagas = 41,
	totalPrivilegiadas = 42,
	verTablaArchivosAbiertos = 43,
	totalPaginasHeap = 44,
	totalAccionesAlocar = 45,
	totalAccionesAclocarEnBytes = 46,
	totalAccionesLiberar = 47,
	totalAccionesLiberarEnBytes = 48,
	verTablaGlobalArchivos = 49,
	modificarGradoMultiprog = 50,
	finalizarProcesoDesdeKernel = 51,
	detenerPlanificacion = 52
};

enum exitCodesEnum{

	FINALIZO_CORRECTAMENTE = 0,
	ERROR_RECURSOS = -1,
	ERROR_ACCESO_ARCHIVO = -2,
	ERROR_PERMISOS = -3,
	ERROR_ESCRITURA = -4,
	ERROR_MEMORIA = -5,
	ERROR_CONSOLA_DESC = -6,
	ERROR_FIN_CONSOLA = -7,
	ERROR_SOLICITUD_HEAP = -8,
	ERROR_ASIGNAR_PAGINAS = -9,
	ERROR_SINDEFINIR = -20

};

enum tipoDeCliente {

	soyConsola = 1,
	soyCPU = 2
};

enum algoritmoElegido {

	SOY_RR = 1,
	SOY_FIFO = 2
};

enum identidad {

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
	bool desconectado;
	bool excepcion;
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

//Prototipos

t_stack* stack_crear();
t_elemento_stack* stack_elemento_crear();
void stack_elemento_destruir(t_elemento_stack* elem);
void stack_destruir(t_stack* stack);
t_elemento_stack* stack_obtener(t_stack* stack, int indice);
t_elemento_stack* stack_head(t_stack* stack);
t_elemento_stack* stack_pop(t_stack* stack);
void stack_push(t_stack* stack, t_elemento_stack* elem);
int stack_tamanio_memoria(t_stack* stack);
int stack_tamanio(t_stack* stack);
t_pedido* stack_proximo_pedido(t_stack* stack, int tamanioPagina, int cantidadPaginasCodigo);
void destruir_PCB(t_PCB* pcb);
char* leerTamanioYMensaje(int sock);
void enviarTamanioYString(int codigoAccion, int sock, char* mensaje);
void enviarTamanioYSerial(int codigoAccion, int sock, int tamanio, char* mensaje);


#endif /* ESTRUCTURASCOMPARTIDAS_H_ */
